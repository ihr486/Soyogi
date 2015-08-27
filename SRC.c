#include "decoder.h"

static DATA_TYPE buf_in[180];
static DATA_TYPE buf_out[160];
int16_t audio[160];
static int in_pos = 16;

void feed_SRC(DATA_TYPE x)
{
    buf_in[in_pos++] = x;
    if(in_pos == 180) {
        for(int i = 0; i < 160; i++) {
            buf_out[i] = 0;
            for(int j = 0; j < 32; j++) {
                buf_out[i] += MUL(buf_in[j + FIR_offset[i]], FIR_coeff[i][j]);
            }
        }
        in_pos = 32;
        for(int i = 0; i < 32; i++) {
            buf_in[i] = buf_in[i + 147];
        }
        for(int i = 0; i < 160; i++) {
            audio[i] = (int16_t)buf_out[i];
        }
        fwrite(audio, sizeof(int16_t) * 160, 1, sox);
    }
}
