#include "decoder.h"

vector_t *vector_list = NULL;

void setup_vectors(void)
{
    vector_list = setup_ref(setup_allocate_natural(sizeof(vector_t) * audio_channels));

    for(int i = 0; i < audio_channels; i++) {
        vector_list[i].right_hand = setup_allocate_natural(sizeof(float) * blocksize[1] / 4);
    }
}

void decouple_square_polar(int n, int magnitude, int angle)
{
    float *M = setup_ref(vector_list[magnitude].body);
    float *A = setup_ref(vector_list[angle].body);

    for(int i = 0; i < n / 2; i++) {
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
