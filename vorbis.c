#include "decoder.h"

uint8_t audio_channels = 0;
static uint32_t audio_sample_rate = 0;
static uint32_t bitrate_maximum = 0;
static uint32_t bitrate_nominal = 0;
static uint32_t bitrate_minimum = 0;
static uint16_t blocksize[2] = {0, 0};
static uint8_t framing_flag = 0;

static int decode_identification_header(void)
{
    uint32_t vorbis_version = read_unsigned_value(32);

    if(vorbis_version != 0) {
        fprintf(stderr, "Unsupported codec version: %u.\n", vorbis_version);
        return 1;
    }

    audio_channels = read_unsigned_value(8);
    audio_sample_rate = read_unsigned_value(32);
    bitrate_maximum = read_unsigned_value(32);
    bitrate_nominal = read_unsigned_value(32);
    bitrate_minimum = read_unsigned_value(32);
    blocksize[0] = 1 << read_unsigned_value(4);
    blocksize[1] = 1 << read_unsigned_value(4);

    if(blocksize[1] > 4096) {
        fprintf(stderr, "Unsupported greater blocksize: %u.\n", blocksize[1]);
        return 1;
    }

    framing_flag = read_unsigned_value(1);

    printf("%d ch, %d SPS, %d bps, %d/%d SPB\n", audio_channels, audio_sample_rate, bitrate_nominal, blocksize[0], blocksize[1]);

    return 0;
}

#define MAX_COMMENT_LENGTH 128

static int decode_comment(char *buf)
{
    uint32_t length = read_unsigned_value(32);

    int i;
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

    for(int i = 0; i < num_comments; i++) {
        decode_comment(comment);

        printf("%s\n", comment);
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

    printf("%d bytes of setup stack consumed.\n", setup_get_head());

    return 0;
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
    }

    return 0;
}
