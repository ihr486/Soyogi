#include "decoder.h"

vector_t *vector_list = NULL;

void setup_vectors(void)
{
    vector_list = setup_ref(setup_allocate_natural(sizeof(vector_t) * audio_channels));

    for(int i = 0; i < audio_channels; i++) {
        vector_list[i].right_hand = setup_allocate_natural(sizeof(float) * blocksize[1] / 4);
        vector_list[i].next_window_flag = 1;

        float *v = setup_ref(vector_list[i].right_hand);

        for(int j = 0; j < blocksize[1] / 4; j++) {
            v[j] = 0.0f;
        }
    }
}

void decouple_square_polar(int n, int magnitude, int angle)
{
    float *M = setup_ref(vector_list[magnitude].body);
    float *A = setup_ref(vector_list[angle].body);

    for(int i = 0; i < n; i++) {
        float new_M, new_A;

        if(M[i] > 0.0f) {
            if(A[i] > 0.0f) {
                new_M = M[i];
                new_A = M[i] - A[i];
            } else {
                new_M = M[i] + A[i];
                new_A = M[i];
            }
        } else {
            if(A[i] > 0.0f) {
                new_M = M[i];
                new_A = M[i] + A[i];
            } else {
                new_M = M[i] - A[i];
                new_A = M[i];
            }
        }

        M[i] = new_M;
        A[i] = new_A;
    }
}

void cache_righthand(int n, int channel, int next_window_flag)
{
    vector_t *vector = &vector_list[channel];

    float *rh = setup_ref(vector->right_hand);
    float *v = setup_ref(vector->body);

    for(int i = 0; i < n / 2; i++) {
        rh[i] = v[i];
    }

    vector->next_window_flag = next_window_flag;
}

void apply_window(float *A, float *B, int n)
{
    const float *win = NULL;

    switch(n) {
    case 32:
        win = vwin32;
        break;
    case 64:
        win = vwin64;
        break;
    case 128:
        win = vwin128;
        break;
    case 256:
        win = vwin256;
        break;
    case 512:
        win = vwin512;
        break;
    case 1024:
        win = vwin1024;
        break;
    case 2048:
        win = vwin2048;
        break;
    default:
        ERROR(ERROR_VORBIS, "Unsupported window size: %d.\n", n * 2);
    }

    for(int i = 0; i < n / 2; i++) {
        float alpha = -win[i] * A[n / 2 - i] - win[n - i] * B[i];
        float beta = -win[n - i] * A[n / 2 - i] + win[i] * B[i];

        A[n / 2 - i] = alpha;
        B[i] = beta;
    }

    for(int i = 0; i < n / 2; i++) {
        float t = A[i];
        A[i] = B[i];
        B[i] = t;
    }
}

void overlap_add(int n, int channel, int previous_window_flag)
{
    vector_t *vector = &vector_list[channel];

    float *rh = setup_ref(vector->right_hand);
    float *v = setup_ref(vector->body);

    if(previous_window_flag && vector->next_window_flag) {
        apply_window(rh, v + n / 2, n);
    } else {
        if(!previous_window_flag) {
            apply_window(rh, v + n / 2, blocksize[0] / 2);
        } else if(!vector->next_window_flag) {
            apply_window(rh + ((blocksize[1] - blocksize[0]) / 4), v + n / 2, n);
        } else {
            ERROR(ERROR_VORBIS, "Illegal overlapping condition.\n");
        }
    }
}
