#include "decoder.h"

static inline void swap(float *A, float *B)
{
    float t = *A;
    *A = *B;
    *B = t;
}

static void FDCT_R_II(float *X, int N_bits);

static void FDCT_R_IV(float *X, int N_bits)
{
    if(N_bits == 0) {
        X[0] *= 0.70710678f;
    } else {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            float c = cosine_table1_1024[N_bits - 1][i];
            float s = sine_table1_1024[N_bits - 1][i];

            float alpha = X[i] * c + X[N - 1 - i] * s;
            float beta = X[i] * s - X[N - 1 - i] * c;

            X[i] = alpha;
            X[N - 1 - i] = beta;
        }

        for(int i = 0; i < N / 4; i++) {
            X[N - 2 - i * 2] = -X[N - 2 - i * 2];
        }

        for(int i = 0; i < N / 4; i++) {
            swap(&X[i + N / 2], &X[N - 1 - i]);
        }

        FDCT_R_II(X, N_bits - 1);
        FDCT_R_II(X + N / 2, N_bits - 1);

        for(int i = 0; i < N / 4; i++) {
            swap(&X[i + N / 2], &X[N - 1 - i]);
        }

        for(int i = 1; i < N / 2; i++) {
            int r = reverse_2048[(reverse_2048[i] >> (11 - N_bits)) - 1] >> (11 - N_bits);

            float alpha = X[i] - X[r];
            float beta = X[i] + X[r];

            X[i] = alpha;
            X[r] = beta;
        }
    }
}

static void FDCT_R_II(float *X, int N_bits)
{
    if(N_bits > 0) {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            float alpha = X[i] + X[N - 1 - i];
            float beta = X[i] - X[N - 1 - i];

            X[i] = alpha;
            X[N - 1 - i] = beta;
        }

        for(int i = 0; i < N / 4; i++) {
            swap(&X[i + N / 2], &X[N - 1 - i]);
        }

        FDCT_R_II(X, N_bits - 1);
        FDCT_R_IV(X + N / 2, N_bits - 1);
    }
}

void FDCT_IV(float *X, int N_bits)
{
    FDCT_R_IV(X, N_bits);

    for(int i = 0; i < (1 << N_bits); i++) {
        int j = reverse_2048[i] >> (11 - N_bits);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }
}
