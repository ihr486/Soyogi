#include "decoder.h"

uint8_t audio_channels = 0;
uint32_t audio_sample_rate = 0;
uint32_t bitrate_nominal = 0;
uint16_t B_N_bits[2] = {0, 0}, B_N[2] = {0, 0};

//int16_t *audio = NULL;

static void decode_identification_header(void)
{
    uint32_t vorbis_version = read_unsigned_value(32);

    if(vorbis_version != 0) {
        ERROR(ERROR_VORBIS, "Unsupported codec version: %u.\n", vorbis_version);
    }

    audio_channels = read_unsigned_value(8);
    audio_sample_rate = read_unsigned_value(32);
    read_unsigned_value(32);
    bitrate_nominal = read_unsigned_value(32);
    read_unsigned_value(32);
    B_N_bits[0] = read_unsigned_value(4);
    B_N_bits[1] = read_unsigned_value(4);

    B_N[0] = 1 << B_N_bits[0];
    B_N[1] = 1 << B_N_bits[1];

    if(B_N[1] > 4096) {
        ERROR(ERROR_VORBIS, "Unsupported greater blocksizes: %u/%u.\n", B_N[0], B_N[1]);
    }

    //audio = (int16_t *)malloc(sizeof(int16_t) * B_N[1] / 2);

    if(!read_unsigned_value(1))
        ERROR(ERROR_SETUP, "Framing error at the end of setup packet.\n");

    INFO("%d ch, %d SPS, %d bps nominal, %d/%d SPB\n", audio_channels, audio_sample_rate, bitrate_nominal, B_N[0], B_N[1]);
}

#define MAX_COMMENT_LENGTH 32768

static int decode_comment(char *buf)
{
    uint32_t length = read_unsigned_value(32);

    unsigned int i;
    for(i = 0; i < length; i++) {
        char c = read_unsigned_value(8);

        if(i < MAX_COMMENT_LENGTH - 1) {
            buf[i] = c;
        }
    }
    if(i >= MAX_COMMENT_LENGTH) {
        i = MAX_COMMENT_LENGTH - 1;
    }
    buf[i] = '\0';
    return i;
}

static void decode_comment_header(void)
{
    char comment[MAX_COMMENT_LENGTH];

    decode_comment(comment);

    //printf("VENDOR=%s\n", comment);

    uint32_t num_comments = read_unsigned_value(32);

    for(unsigned int i = 0; i < num_comments; i++) {
        decode_comment(comment);

        //printf("%s\n", comment);
    }
}

static void decode_setup_header(void)
{
    setup_codebooks();

    int time_count = read_unsigned_value(6) + 1;

    for(int i = 0; i < time_count; i++) {
        uint16_t dummy = read_unsigned_value(16);

        if(dummy) {
            ERROR(ERROR_VORBIS, "Time domain transforms are not used in Vorbis I.\n");
        }
    }

    setup_floors();

    setup_residues();

    setup_mappings();

    setup_modes();

    setup_vectors();

    INFO("%d bytes of setup stack consumed.\n", setup_get_head());
}

void decode_audio_packet(void)
{
    double FDCT_time = 0, residue_time = 0;

    double initial_clock = get_us();

    int mode_number = read_unsigned_value_PF(ilog(mode_num - 1));

    vorbis_mode_t *mode = &mode_list[mode_number];

    int V_N_bits = B_N_bits[mode->blockflag] - 1;
    int V_N = 1 << V_N_bits;

    int previous_window_flag = 1, next_window_flag = 1;

    if(mode->blockflag) {
        previous_window_flag = read_bit_PF();
        next_window_flag = read_bit_PF();
    }

    mapping_header_t *mapping = &mapping_list[mode->mapping];
    channel_t *channel_list = setup_ref(mapping->channel_list);
    submap_t *submap_list = setup_ref(mapping->submap_list);

    uint16_t setup_origin = setup_get_head();

    for(int i = 0; i < audio_channels; i++) {
        int submap_number = 0;
        if(mapping->submaps > 1)
            submap_number = channel_list[i].mux;

        int floor_number = submap_list[submap_number].floor;

        decode_floor1(floor_number, i);

        vector_list[i].no_residue = !vector_list[i].nonzero;
    }

    coupling_step_t *step_list = setup_ref(mapping->coupling_step_list);
    for(int i = 0; i < mapping->coupling_steps; i++) {
        if(vector_list[step_list[i].magnitude].nonzero || !vector_list[step_list[i].angle].nonzero) {
            vector_list[step_list[i].magnitude].no_residue = 0;
            vector_list[step_list[i].angle].no_residue = 0;
        }
    }

    if(mapping->submaps == 1) {
        submap_t *submap = setup_ref(mapping->submap_list);
        int residue_number = submap->residue;

        for(int i = 0; i < audio_channels; i++) {
            vector_list[i].do_not_decode_flag = vector_list[i].no_residue;
        }

        double residue_entry = get_us();

        decode_residue(V_N_bits, residue_number, 0, audio_channels);

        residue_time += get_us() - residue_entry;
    } else {
        ERROR(ERROR_VORBIS, "Multiple submaps are not supported yet.\n");
    }

    for(int i = 0; i < mapping->coupling_steps; i++) {
        decouple_square_polar(V_N, step_list[i].magnitude, step_list[i].angle);
    }

    for(int i = 0; i < audio_channels; i++) {
        DATA_TYPE *v = setup_ref(vector_list[i].body);

        if(vector_list[i].nonzero) {
            synthesize_floor1(V_N, i);
        }

        double FDCT_entry = get_us();

        FDCT_IV(v, V_N_bits);

        FDCT_time += get_us() - FDCT_entry;

        for(int j = 0; j < V_N; j++) {
#ifndef FIXED_POINT
            v[j] *= 16000.0f;
#endif
        }

        overlap_add(V_N_bits, i, previous_window_flag);
    }

    DATA_TYPE *v_out = setup_ref(vector_list[0].body);
    DATA_TYPE *rh = setup_ref(vector_list[0].right_hand);
    /*if(previous_window_flag && vector_list[0].next_window_flag) {
        for(int i = 0; i < V_N / 2; i++) {
            audio[i + V_N / 2] = (int16_t)rh[i];
            audio[i] = (int16_t)v_out[i + V_N / 2];
        }
    } else {
        if(!previous_window_flag) {
            for(int i = 0; i < B_N[0] / 4; i++) {
                audio[i] = (int16_t)v_out[B_N[1] / 4 + (B_N[1] - B_N[0]) / 4 + i];
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                audio[B_N[0] / 4 + i] = (int16_t)rh[i];
            }
            for(int i = 0; i < (B_N[1] - B_N[0]) / 4; i++) {
                audio[B_N[0] / 2 + i] = (int16_t)(-v_out[B_N[1] / 4 + (B_N[1] - B_N[0]) / 4 - 1 - i]);
            }
        } else if(!vector_list[0].next_window_flag) {
            for(int i = 0; i < (B_N[1] - B_N[0]) / 4; i++) {
                audio[i] = (int16_t)(-rh[B_N[1] / 4 - 1 - i]);
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                audio[(B_N[1] - B_N[0]) / 4 + i] = (int16_t)v_out[i + B_N[0] / 4];
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                audio[B_N[1] / 4 + i] = (int16_t)rh[i];
            }
        }
    }*/

    if(previous_window_flag && vector_list[0].next_window_flag) {
        for(int i = 0; i < V_N / 2; i++) {
            feed_SRC(v_out[i + V_N / 2]);
        }
        for(int i = 0; i < V_N / 2; i++) {
            feed_SRC(rh[i]);
        }
    } else {
        if(!previous_window_flag) {
            for(int i = 0; i < B_N[0] / 4; i++) {
                feed_SRC(v_out[B_N[1] / 4 + (B_N[1] - B_N[0]) / 4 + i]);
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                feed_SRC(rh[i]);
            }
            for(int i = 0; i < (B_N[1] - B_N[0]) / 4; i++) {
                feed_SRC(-v_out[B_N[1] / 4 + (B_N[1] - B_N[0]) / 4 - 1 - i]);
            }
        } else if(!vector_list[0].next_window_flag) {
            for(int i = 0; i < (B_N[1] - B_N[0]) / 4; i++) {
                feed_SRC(-rh[B_N[1] / 4 - 1 - i]);
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                feed_SRC(v_out[i + B_N[0] / 4]);
            }
            for(int i = 0; i < B_N[0] / 4; i++) {
                feed_SRC(rh[i]);
            }
        }
    }

    //int this_window_flag = vector_list[0].next_window_flag;

    for(int i = 0; i < audio_channels; i++) {
        cache_righthand(V_N, i, next_window_flag);
    }
    setup_set_head(setup_origin);

    double packet_time = get_us() - initial_clock;
    printf("%lf %lf %lf\n", packet_time, FDCT_time, residue_time);

    /*if(previous_window_flag && this_window_flag) {
        fwrite(audio, sizeof(int16_t) * V_N, 1, sox);
    } else {
        fwrite(audio, sizeof(int16_t) * (B_N[0] + B_N[1]) / 4, 1, sox);
    }*/
}

void decode_packet(void)
{
    if(read_unsigned_value(1)) {
        uint8_t header_type = read_unsigned_value(7);

        if(read_unsigned_value(8) != 'v' ||
           read_unsigned_value(8) != 'o' ||
           read_unsigned_value(8) != 'r' ||
           read_unsigned_value(8) != 'b' ||
           read_unsigned_value(8) != 'i' ||
           read_unsigned_value(8) != 's') {
            ERROR(ERROR_VORBIS, "Corrupted header signature.\n");
        }

        switch(header_type) {
        case 0:
            decode_identification_header();
            break;
        case 1:
            decode_comment_header();
            break;
        case 2:
            decode_setup_header();
            break;
        default:
            ERROR(ERROR_VORBIS, "Illegal header type: %u.\n", header_type);
        }
    } else {
        prefetch_packet(1);

        decode_audio_packet();
    }
}
