#include "decoder.h"

vector_t *vector_list = NULL;

void setup_vectors(void)
{
    vector_list = setup_ref(setup_allocate_natural(sizeof(vector_t) * audio_channels));

    for(int i = 0; i < audio_channels; i++) {
        vector_list[i].right_hand = setup_allocate_natural(sizeof(float) * B_N[1] / 4);
        vector_list[i].next_window_flag = 1;

        float *v = setup_ref(vector_list[i].right_hand);

        for(int j = 0; j < B_N[1] / 4; j++) {
            v[j] = 0.0f;
        }
    }
}

void decouple_square_polar(int V_N, int magnitude, int angle)
{
    float *M = setup_ref(vector_list[magnitude].body);
    float *A = setup_ref(vector_list[angle].body);

    for(int i = 0; i < V_N; i++) {
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

void cache_righthand(int V_N, int channel, int next_window_flag)
{
    vector_t *vector = &vector_list[channel];

    float *rh = setup_ref(vector->right_hand);
    float *v = setup_ref(vector->body);

    for(int i = 0; i < V_N / 2; i++) {
        rh[i] = v[i];
    }

    vector->next_window_flag = next_window_flag;
}

void apply_window(float *A, float *B, int V_N_bits)
{
    int V_N = 1 << V_N_bits;

    const float *win = vwin32_2048[V_N_bits - 5];

    for(int i = 0; i < V_N / 2; i++) {
        float alpha = -win[i] * A[V_N / 2 - 1 - i] - win[V_N - 1 - i] * B[i];
        float beta = -win[V_N - 1 - i] * A[V_N / 2 - 1 - i] + win[i] * B[i];

        A[V_N / 2 - 1 - i] = alpha;
        B[i] = beta;
    }

    for(int i = 0; i < V_N / 2; i++) {
        float t = A[i];
        A[i] = B[i];
        B[i] = t;
    }
}

void overlap_add(int V_N_bits, int channel, int previous_window_flag)
{
    vector_t *vector = &vector_list[channel];

    float *rh = setup_ref(vector->right_hand);
    float *v = setup_ref(vector->body);

    int V_N = 1 << V_N_bits;

    if(previous_window_flag && vector->next_window_flag) {
        apply_window(rh, v + V_N / 2, V_N_bits);
    } else {
        if(!previous_window_flag) {
            apply_window(rh, v + V_N / 2, B_N_bits[0] - 1);
        } else if(!vector->next_window_flag) {
            apply_window(rh + ((B_N[1] - B_N[0]) / 4), v + V_N / 2, V_N_bits);
        } else {
            ERROR(ERROR_VORBIS, "Illegal overlapping condition.\n");
        }
    }
}
