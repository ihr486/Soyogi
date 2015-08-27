#include "decoder.h"

vector_t *vector_list = NULL;

void setup_vectors(void)
{
    vector_list = setup_ref(setup_allocate_natural(sizeof(vector_t) * audio_channels));

    for(int i = 0; i < audio_channels; i++) {
        vector_list[i].right_hand = setup_allocate_natural(sizeof(DATA_TYPE) * B_N[1] / 4);
        vector_list[i].next_window_flag = 1;

        DATA_TYPE *v = setup_ref(vector_list[i].right_hand);

        memset(v, 0, sizeof(DATA_TYPE) * B_N[1] / 4);
    }
}

void decouple_square_polar(int V_N, int magnitude, int angle)
{
    DATA_TYPE *M = setup_ref(vector_list[magnitude].body);
    DATA_TYPE *A = setup_ref(vector_list[angle].body);

    for(int i = 0; i < V_N; i++) {
        DATA_TYPE new_M, new_A;

        if(M[i] > 0) {
            if(A[i] > 0) {
                new_M = M[i];
                new_A = M[i] - A[i];
            } else {
                new_M = M[i] + A[i];
                new_A = M[i];
            }
        } else {
            if(A[i] > 0) {
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

    DATA_TYPE *rh = setup_ref(vector->right_hand);
    DATA_TYPE *v = setup_ref(vector->body);

    for(int i = 0; i < V_N / 2; i++) {
        rh[i] = v[i];
    }

    vector->next_window_flag = next_window_flag;
}

void apply_window(DATA_TYPE *A, DATA_TYPE *B, int V_N_bits)
{
    int V_N = 1 << V_N_bits;

    const UNSIGNED_COEFF_TYPE *win = vwin32_2048[V_N_bits - 5];

    for(int i = 0; i < V_N / 2; i++) {
        DATA_TYPE alpha = -MUL(A[V_N / 2 - 1 - i], win[i]) - MUL(B[i], win[V_N - 1 - i]);
        DATA_TYPE beta = -MUL(A[V_N / 2 - 1 - i], win[V_N - 1 - i]) + MUL(B[i], win[i]);

        A[V_N / 2 - 1 - i] = alpha;
        B[i] = beta;
    }
}

void overlap_add(int V_N_bits, int channel, int previous_window_flag)
{
    vector_t *vector = &vector_list[channel];

    DATA_TYPE *rh = setup_ref(vector->right_hand);
    DATA_TYPE *v = setup_ref(vector->body);

    int V_N = 1 << V_N_bits;

    if(previous_window_flag && vector->next_window_flag) {
        apply_window(rh, v + V_N / 2, V_N_bits);
    } else {
        if(!previous_window_flag) {
            apply_window(rh, v + V_N / 2 + (B_N[1] - B_N[0]) / 4, B_N_bits[0] - 1);
        } else if(!vector->next_window_flag) {
            apply_window(rh, v + V_N / 2, V_N_bits);
        } else {
            ERROR(ERROR_VORBIS, "Illegal overlapping condition.\n");
        }
    }
}
