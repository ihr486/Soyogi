#include "decoder.h"

uint8_t audio_channels = 0;
uint32_t audio_sample_rate = 0;
uint32_t bitrate_nominal = 0;
uint16_t blocksize[2] = {0, 0};

static int decode_identification_header(void)
{
    uint32_t vorbis_version = read_unsigned_value(32);

    if(vorbis_version != 0) {
        fprintf(stderr, "Unsupported codec version: %u.\n", vorbis_version);
        return 1;
    }

    audio_channels = read_unsigned_value(8);
    audio_sample_rate = read_unsigned_value(32);
    read_unsigned_value(32);
    bitrate_nominal = read_unsigned_value(32);
    read_unsigned_value(32);
    blocksize[0] = 1 << read_unsigned_value(4);
    blocksize[1] = 1 << read_unsigned_value(4);

    if(blocksize[1] > 4096) {
        fprintf(stderr, "Unsupported greater blocksize: %u.\n", blocksize[1]);
        return 1;
    }

    if(!read_unsigned_value(1))
        ERROR(ERROR_SETUP, "Framing error at the end of setup packet.\n");

    printf("%d ch, %d SPS, %d bps, %d/%d SPB\n", audio_channels, audio_sample_rate, bitrate_nominal, blocksize[0], blocksize[1]);

    return 0;
}

#define MAX_COMMENT_LENGTH 16384

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

static int decode_comment_header(void)
{
    char comment[MAX_COMMENT_LENGTH];

    decode_comment(comment);

    printf("VENDOR=%s\n", comment);

    uint32_t num_comments = read_unsigned_value(32);

    for(unsigned int i = 0; i < num_comments; i++) {
        decode_comment(comment);

        //printf("%s\n", comment);
    }
    return 0;
}

static int decode_setup_header(void)
{
    setup_codebooks();

    int time_count = read_unsigned_value(6) + 1;

    for(int i = 0; i < time_count; i++) {
        uint16_t dummy = read_unsigned_value(16);

        if(dummy) {
            fprintf(stderr, "Time domain transforms are not used in Vorbis I.\n");
            return 1;
        }
    }

    printf("%d time domain transforms decoded.\n", time_count);

    setup_floors();

    setup_residues();

    setup_mappings();

    setup_modes();

    setup_vectors();

    printf("%d bytes of setup stack consumed.\n", setup_get_head());

    return 0;
}

void decode_audio_packet(void)
{
    int mode_number = read_unsigned_value(ilog(mode_num - 1));

    vorbis_mode_t *mode = &mode_list[mode_number];

    int n = blocksize[mode->blockflag];

    int previous_window_flag = 1, next_window_flag = 1;

    if(mode->blockflag) {
        previous_window_flag = read_unsigned_value(1);
        next_window_flag = read_unsigned_value(1);
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

        //INFO("Decoding %d residue vectors according to submap %d...\n", audio_channels, 0);

        decode_residue(n, residue_number, 0, audio_channels);
    } else {
        ERROR(ERROR_VORBIS, "Multiple submaps are not supported yet.\n");
    }

    for(int i = 0; i < mapping->coupling_steps; i++) {
        decouple_square_polar(n / 2, step_list[i].magnitude, step_list[i].angle);
    }

    for(int i = 0; i < audio_channels; i++) {
        float *v = setup_ref(vector_list[i].body);

        if(vector_list[i].nonzero) {
            synthesize_floor1(n / 2, i);
        }

        FDCT_IV(v, n / 2);

        for(int j = 0; j < n / 2; j++) {
            v[j] *= 3000.f;
        }

        overlap_add(n / 2, i, previous_window_flag);
    }

    int16_t *audio = (int16_t *)malloc(sizeof(int16_t) * blocksize[1] / 2);

    float *v_out = setup_ref(vector_list[0].body);
    float *rh = setup_ref(vector_list[0].right_hand);
    if(previous_window_flag && vector_list[0].next_window_flag) {
        for(int i = 0; i < n / 4; i++) {
            audio[i] = (int16_t)rh[i];
            audio[i + n / 4] = (int16_t)v_out[i + n / 4];
        }
        //fwrite(audio, sizeof(int16_t) * n / 2, 1, output);
        int error;
        pa_simple_write(pulse_ctx, audio, sizeof(int16_t) * n / 2, &error);
    } else {
        if(!previous_window_flag) {
            for(int i = 0; i < blocksize[0] / 4; i++) {
                audio[i] = (int16_t)rh[i];
            }
            for(int i = 0; i < blocksize[1] / 4; i++) {
                audio[blocksize[0] / 4 + i] = (int16_t)v_out[i + blocksize[1] / 4];
            }
        } else if(!vector_list[0].next_window_flag) {
            for(int i = 0; i < blocksize[1] / 4; i++) {
                audio[i] = (int16_t)rh[i];
            }
            for(int i = 0; i < blocksize[0] / 4; i++) {
                audio[blocksize[1] / 4 + i] = (int16_t)v_out[i + blocksize[0] / 4];
            }
        }
        //fwrite(audio, sizeof(int16_t) * (blocksize[0] + blocksize[1]) / 4, 1, output);
        int error;
        pa_simple_write(pulse_ctx, audio, sizeof(int16_t) * (blocksize[0] + blocksize[1]) / 4, &error);
    }

    free(audio);

    for(int i = 0; i < audio_channels; i++) {
        cache_righthand(n / 2, i, next_window_flag);
    }
    setup_set_head(setup_origin);
}

int decode_packet(void)
{
    if(read_unsigned_value(1)) {
        uint8_t header_type = read_unsigned_value(7);

        if(read_unsigned_value(8) != 'v' ||
           read_unsigned_value(8) != 'o' ||
           read_unsigned_value(8) != 'r' ||
           read_unsigned_value(8) != 'b' ||
           read_unsigned_value(8) != 'i' ||
           read_unsigned_value(8) != 's') {
            fprintf(stderr, "Corrupted header signature.\n");
            return 1;
        }

        switch(header_type) {
        case 0:
            return decode_identification_header();
        case 1:
            return decode_comment_header();
        case 2:
            return decode_setup_header();
        default:
            fprintf(stderr, "Illegal header type: %u.\n", header_type);
            return 1;
        }
    } else {
        decode_audio_packet();
    }

    return 0;
}
