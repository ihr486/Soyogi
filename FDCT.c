#include "decoder.h"

static inline int reverse_bits(int n, int l)
{
    int ret = 0;
    while(l--) {
        ret = (ret << 1) | (n & 1);
        n >>= 1;
    }
    return ret;
}

static inline void swap(float *A, float *B)
{
    float t = *A;
    *A = *B;
    *B = t;
}

static void FDCT_R_IV(float *X, int V_N_bits);

static void FDCT_R_III(float *X, int V_N_bits)
{
    int half_V_N = 1 << (V_N_bits - 1);

    if(V_N_bits) {
        FDCT_R_III(X, V_N_bits - 1);
        FDCT_R_IV(X + half_V_N, V_N_bits - 1);

        for(int i = 0; i < half_V_N; i++) {
            float alpha = X[i] + X[i + half_V_N];
            float beta = X[i] - X[i + half_V_N];

            X[i] = alpha;
            X[i + half_V_N] = beta;
        }
        for(int i = 0; i < half_V_N / 2; i++) {
            swap(&X[i + half_V_N], &X[(1 << V_N_bits) - 1 - i]);
        }
    }
}

static const float PI = 3.1415926f;

static void FDCT_R_IV(float *X, int V_N_bits)
{
    int half_V_N = 1 << (V_N_bits - 1);

    if(!V_N_bits) {
        X[0] *= 0.707106781f;
    } else {
        FDCT_R_III(X, V_N_bits - 1);
        FDCT_R_IV(X + half_V_N, V_N_bits - 1);

        for(int i = 0; i < half_V_N; i++) {
            float alpha = X[i] + X[i + half_V_N];
            float beta = X[i] - X[i + half_V_N];

            X[i] = alpha * half_secant_table1_1024[V_N_bits - 1][i];
            X[i + half_V_N] = beta * half_cosecant_table1_1024[V_N_bits - 1][i];
        }
        for(int i = 0; i < half_V_N / 2; i++) {
            swap(&X[i + half_V_N], &X[(1 << V_N_bits) - 1 - i]);
        }
    }
}

void FDCT_IV(float *X, int V_N_bits)
{
    int V_N = 1 << V_N_bits;

    for(int i = V_N - 1; i > 0; i--) {
        X[i] += X[i - 1];
    }
    for(int i = 1; i < V_N_bits; i++) {
        for(int j = 0; j < (1 << (i - 1)); j++) {
            for(int k = 0; k < (1 << (V_N_bits - i)) - 1; k++) {
                X[V_N - 1 - (k << i) - j] += X[V_N - 1 - ((k + 1) << i) - j];
            }
        }
    }
    for(int i = 0; i < V_N; i++) {
        int j = reverse_bits(i, V_N_bits);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }

    FDCT_R_IV(X, V_N_bits);
}
