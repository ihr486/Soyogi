#include "decoder.h"

//FILE *output = NULL;
pa_simple *pulse_ctx = NULL;

static FILE *g_source = NULL;
static uint8_t buf[SECTOR_SIZE];
static int buf_depth = 0, buf_head = 0;
static bool EOF_flag = false;

bool reached_EOF(void)
{
    return EOF_flag;
}

uint8_t read_unsigned_byte(void)
{
    if(buf_head >= buf_depth) {
        if(EOF_flag) {
            return 0;
        } else {
            buf_depth = fread(buf, 1, SECTOR_SIZE, g_source);
            buf_head = 0;
            if(buf_depth < SECTOR_SIZE) {
                EOF_flag = true;
            }
        }
    }
    return buf[buf_head++];
}

uint16_t read_unsigned_short(void)
{
    return (uint16_t)read_unsigned_byte() | ((uint16_t)read_unsigned_byte() << 8);
}

uint32_t read_unsigned_long(void)
{
    return (uint32_t)read_unsigned_short() | ((uint32_t)read_unsigned_short() << 16);
}

uint64_t read_unsigned_long_long(void)
{
    return (uint64_t)read_unsigned_long() | ((uint64_t)read_unsigned_long() << 32);
}

static const pa_sample_spec ss = {
    .format = PA_SAMPLE_S16LE,
    .rate = 44100,
    .channels = 1
};

int main(int argc, const char *argv[])
{
    if(argc != 2) {
        fprintf(stderr, "Please specify at least one input file.\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb");
    if(!fp) {
        fprintf(stderr, "Failed to open %s.\n", argv[1]);
        return 1;
    }

    /*output = popen("/usr/bin/play -t raw -r 44100 -c 1 -e s -b 16 - > /dev/null 2>&1", "w");
    if(!output) {
        fprintf(stderr, "Failed to connect to sox.\n");
        return 1;
    }*/

    int error;
    pulse_ctx = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "Vorbis playback", &ss, NULL, NULL, &error);
    if(!pulse_ctx) {
        fprintf(stderr, "Failed to connect to Pulseaudio.\n");
        return 1;
    }

    g_source = fp;

    int ret;
    if(!(ret = setjmp(jump_env))) {
        decode();
    } else {
        printf("Critical error detected during decode process.\n");
    }

    pa_simple_free(pulse_ctx);

    //pclose(output);
    fclose(fp);

    return 0;
}
