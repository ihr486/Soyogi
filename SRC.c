#include "decoder.h"

static DATA_TYPE left_in[180], right_in[180];
static DATA_TYPE left_out[160], right_out[160];
static int16_t audio[320];
static int in_pos = 16;

void feed_SRC(DATA_TYPE left, DATA_TYPE right)
{
    left_in[in_pos] = left;
    right_in[in_pos] = right;
    if(++in_pos == 180) {
        for(int i = 0; i < 160; i++) {
            left_out[i] = 0;
            right_out[i] = 0;
            for(int j = 0; j < 32; j++) {
                left_out[i] += MUL(left_in[j + FIR_offset[i]], FIR_coeff[i][j]);
                right_out[i] += MUL(right_in[j + FIR_offset[i]], FIR_coeff[i][j]);
            }
        }
        in_pos = 33;
        for(int i = 0; i < 33; i++) {
            left_in[i] = left_in[i + 147];
            right_in[i] = right_in[i + 147];
        }
        for(int i = 0; i < 160; i++) {
            audio[i * 2] = clamp16s((int)left_out[i]);
            audio[i * 2 + 1] = clamp16s((int)right_out[i]);
        }
        fwrite(audio, sizeof(int16_t) * 320, 1, sox);
    }
}
