#include "decoder.h"

inline void swap(FIX *x, FIX *y)
{
    FIX t = *x;
    *x = *y;
    *y = t;
}

int reverse_bits(int n, int m)
{
    int ret = 0;
    while(m--) {
        ret = (ret << 1) | (n & 1);
        n >>= 1;
    }
    return ret;
}

void FDCT_R_II(FIX *X, int N_bits);

void FDCT_R_IV(FIX *X, int N_bits)
{
    if(N_bits == 0) {
        X[0] = FIX_MUL32(X[0], 0xB504F333);
    } else {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            uint32_t c = cosine_table1_1024[N_bits - 1][i];
            uint32_t s = sine_table1_1024[N_bits - 1][i];

            FIX alpha = FIX_MUL32(X[i], c) + FIX_MUL32(X[N - 1 - i], s);
            FIX beta = FIX_MUL32(X[i], s) - FIX_MUL32(X[N - 1 - i], c);

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

            FIX alpha = X[i] - X[r];
            FIX beta = X[i] + X[r];

            X[i] = alpha;
            X[r] = beta;
        }
    }
}

void FDCT_R_II(FIX *X, int N_bits)
{
    if(N_bits > 0) {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            FIX alpha = X[i] + X[N - 1 - i];
            FIX beta = X[i] - X[N - 1 - i];

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

void FDCT_IV(FIX *X, int N_bits)
{
    int N = 1 << N_bits;
    
    FDCT_R_IV(X, N_bits);

    for(int i = 0; i < N; i++) {
        int j = reverse_2048[i] >> (11 - N_bits);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }
}

