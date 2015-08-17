#include "decoder.h"

static inline void swap(float *A, float *B)
{
    float t = *A;
    *A = *B;
    *B = t;
}

static inline void FDCT_8(float *X)
{
    X[1] *= 0.7071067f;
    X[3] *= 0.7071067f;
    X[5] *= 0.7071067f;
    X[7] *= 0.7071067f;

    register float alpha, beta;
    alpha = X[0] + X[1], beta = X[0] - X[1];
    X[0] = alpha, X[1] = beta;
    alpha = X[2] + X[3], beta = X[2] - X[3];
    X[2] = alpha * 0.5411961f;
    X[3] = beta * 1.306563f;
    alpha = X[4] + X[5], beta = X[4] - X[5];
    X[4] = alpha, X[5] = beta;
    alpha = X[6] + X[7], beta = X[6] - X[7];
    X[6] = alpha * 0.5411961f;
    X[7] = beta * 1.306563f;

    alpha = X[0] + X[2], beta = X[0] - X[2];
    X[0] = alpha, X[2] = beta;
    alpha = X[1] + X[3], beta = X[1] - X[3];
    X[1] = alpha, X[3] = beta;
    alpha = X[4] + X[6], beta = X[4] - X[6];
    X[4] = alpha * 0.5097956f;
    X[6] = beta * 2.562915f;
    alpha = X[5] + X[7], beta = X[5] - X[7];
    X[5] = alpha * 0.6013449f;
    X[7] = beta * 0.8999762f;

    alpha = X[2], beta = X[6];
    X[2] = X[3], X[6] = X[7];
    X[3] = alpha, X[7] = beta;
}

static void FDCT_R_IV(float *X, int V_N_bits);

static void FDCT_R_III(float *X, int V_N_bits)
{
    int half_V_N = 1 << (V_N_bits - 1);

    if(V_N_bits > 3) {
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
    } else {
        FDCT_8(X);

        register float alpha, beta;
        alpha = X[0] + X[4], beta = X[0] - X[4];
        X[0] = alpha, X[4] = beta;
        alpha = X[1] + X[5], beta = X[1] - X[5];
        X[1] = alpha, X[5] = beta;
        alpha = X[2] + X[6], beta = X[2] - X[6];
        X[2] = alpha, X[6] = beta;
        alpha = X[3] + X[7], beta = X[3] - X[7];
        X[3] = alpha, X[7] = beta;

        alpha = X[4], beta = X[5];
        X[4] = X[7], X[5] = X[6];
        X[7] = alpha, X[6] = beta;
    }
}

static void FDCT_R_IV(float *X, int V_N_bits)
{
    int half_V_N = 1 << (V_N_bits - 1);

    if(V_N_bits > 3) {
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
    } else {
        FDCT_8(X);

        float alpha, beta;
        alpha = X[0] + X[4], beta = X[0] - X[4];
        X[0] = alpha * 0.5024193f;
        X[4] = beta * 5.101149f;
        alpha = X[1] + X[5], beta = X[1] - X[5];
        X[1] = alpha * 0.5224986f;
        X[5] = beta * 1.722447f;
        alpha = X[2] + X[6], beta = X[2] - X[6];
        X[2] = alpha * 0.5669441f;
        X[6] = beta * 1.060678f;
        alpha = X[3] + X[7], beta = X[3] - X[7];
        X[3] = alpha * 0.6468218f;
        X[7] = beta * 0.7881546f;

        alpha = X[4], beta = X[5];
        X[4] = X[7], X[5] = X[6];
        X[7] = alpha, X[6] = beta;
    }
}

void __attribute__((hot)) FDCT_IV(float *X, int V_N_bits)
{
    int V_N = 1 << V_N_bits;

    for(int i = V_N - 1; i > 0; i--) {
        X[i] += X[i - 1];
    }
    for(int i = V_N - 1; i > 1; i -= 2) {
        X[i] += X[i - 2];
    }
    for(int i = V_N - 1; i > 3; i -= 4) {
        X[i] += X[i - 4];
        X[i - 1] += X[i - 5];
    }
    for(int i = V_N - 1; i > 7; i -= 8) {
        X[i] += X[i - 8];
        X[i - 1] += X[i - 9];
        X[i - 2] += X[i - 10];
        X[i - 3] += X[i - 11];
    }
    for(int i = 4; i < V_N_bits - 3; i++) {
        for(int j = 0; j < (1 << (i - 1)); j++) {
            float prev = 0.0f;
            for(int k = j | (1 << (i - 1)); k < V_N; k += (1 << i)) {
                float temp = X[k];
                X[k] += prev;
                prev = temp;
            }
        }
    }
    for(int i = 0; i < V_N / 16; i++) {
        X[(15 << (V_N_bits - 4)) + i] += X[(13 << (V_N_bits - 4)) + i];
        X[(13 << (V_N_bits - 4)) + i] += X[(11 << (V_N_bits - 4)) + i];
        X[(11 << (V_N_bits - 4)) + i] += X[(9 << (V_N_bits - 4)) + i];
        X[(9 << (V_N_bits - 4)) + i] += X[(7 << (V_N_bits - 4)) + i];
        X[(7 << (V_N_bits - 4)) + i] += X[(5 << (V_N_bits - 4)) + i];
        X[(5 << (V_N_bits - 4)) + i] += X[(3 << (V_N_bits - 4)) + i];
        X[(3 << (V_N_bits - 4)) + i] += X[(1 << (V_N_bits - 4)) + i];
    }
    for(int i = 0; i < V_N / 8; i++) {
        X[(7 << (V_N_bits - 3)) + i] += X[(5 << (V_N_bits - 3)) + i];
        X[(5 << (V_N_bits - 3)) + i] += X[(3 << (V_N_bits - 3)) + i];
        X[(3 << (V_N_bits - 3)) + i] += X[(1 << (V_N_bits - 3)) + i];
    }
    for(int i = 0; i < V_N; i++) {
        int j = reverse_2048[i] >> (11 - V_N_bits);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }

    FDCT_R_IV(X, V_N_bits);
}
