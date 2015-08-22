#include "decoder.h"

static inline void swap(float *A, float *B)
{
    float t = *A;
    *A = *B;
    *B = t;
}

const float PI = 3.141592653589793f;

static inline int reverse_bits(int n, int m)
{
    int ret = 0;
    while(m--) {
        ret = (ret << 1) | (n & 1);
        n >>= 1;
    }
    return ret;
}

static void FDCT_R_II(float *X, int N_bits);

static void FDCT_R_IV(float *X, int N_bits)
{
    if(N_bits == 0) {
        X[0] *= cos(0.25f * PI);
    } else {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            float alpha = X[i] * cos(0.5f * PI / N * (i + 0.5f)) + X[N - 1 - i] * sin(0.5f * PI / N * (i + 0.5f));
            float beta = X[i] * sin(0.5f * PI / N * (i + 0.5f)) - X[N - 1 - i] * cos(0.5f * PI / N * (i + 0.5f));

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
            float alpha = X[i] - X[reverse_bits(reverse_bits(i, N_bits) - 1, N_bits)];
            float beta = X[i] + X[reverse_bits(reverse_bits(i, N_bits) - 1, N_bits)];

            X[i] = alpha;
            X[reverse_bits(reverse_bits(i, N_bits) - 1, N_bits)] = beta;
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
        int j = reverse_bits(i, N_bits);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }
}
