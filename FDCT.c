#include "decoder.h"

inline void swap(FIX *x, FIX *y)
{
    FIX t = *x;
    *x = *y;
    *y = t;
}

void FDCT_R_II(FIX *X, int N_bits);

void FDCT_R_IV(FIX *X, int N_bits)
{
    if(N_bits == 3) {
        register FIX alpha, beta;
        alpha = FIX_MUL32(X[0], cosine_table1_1024[2][0]) + FIX_MUL32(X[7], sine_table1_1024[2][0]);
        beta = FIX_MUL32(X[0], sine_table1_1024[2][0]) - FIX_MUL32(X[7], cosine_table1_1024[2][0]);

        X[0] = alpha, X[7] = beta;

        alpha = FIX_MUL32(X[1], cosine_table1_1024[2][1]) + FIX_MUL32(X[6], sine_table1_1024[2][1]);
        beta = FIX_MUL32(X[1], sine_table1_1024[2][1]) - FIX_MUL32(X[6], cosine_table1_1024[2][1]);

        X[1] = alpha, X[6] = -beta;

        alpha = FIX_MUL32(X[2], cosine_table1_1024[2][2]) + FIX_MUL32(X[5], sine_table1_1024[2][2]);
        beta = FIX_MUL32(X[2], sine_table1_1024[2][2]) - FIX_MUL32(X[5], cosine_table1_1024[2][2]);

        X[2] = alpha, X[5] = beta;

        alpha = FIX_MUL32(X[3], cosine_table1_1024[2][3]) + FIX_MUL32(X[4], sine_table1_1024[2][3]);
        beta = FIX_MUL32(X[3], sine_table1_1024[2][3]) - FIX_MUL32(X[4], cosine_table1_1024[2][3]);

        X[3] = alpha, X[4] = -beta;

        alpha = X[4], X[4] = X[7], X[7] = alpha;

        alpha = X[5], X[5] = X[6], X[6] = alpha;

        alpha = X[0] + X[3], beta = X[0] - X[3];
        X[0] = alpha, X[3] = beta;

        alpha = X[1] + X[2], beta = X[1] - X[2];
        X[1] = alpha, X[2] = beta;

        alpha = X[4] + X[7], beta = X[4] - X[7];
        X[4] = alpha, X[7] = beta;

        alpha = X[5] + X[6], beta = X[5] - X[6];
        X[5] = alpha, X[6] = beta;

        alpha = X[2], X[2] = X[3], X[3] = alpha;

        alpha = X[6], X[6] = X[7], X[7] = alpha;

        alpha = X[0] + X[1], beta = X[0] - X[1];
        X[0] = alpha, X[1] = beta;

        alpha = FIX_MUL32(X[2], cosine_table1_1024[0][0]) + FIX_MUL32(X[3], sine_table1_1024[0][0]);
        beta = FIX_MUL32(X[2], sine_table1_1024[0][0]) - FIX_MUL32(X[3], cosine_table1_1024[0][0]);
        X[2] = alpha, X[3] = beta;

        alpha = X[4] + X[5], beta = X[4] - X[5];
        X[4] = alpha, X[5] = beta;

        alpha = FIX_MUL32(X[6], cosine_table1_1024[0][0]) + FIX_MUL32(X[7], sine_table1_1024[0][0]);
        beta = FIX_MUL32(X[6], sine_table1_1024[0][0]) - FIX_MUL32(X[7], cosine_table1_1024[0][0]);
        X[6] = alpha, X[7] = beta;

        X[1] = FIX_MUL32(X[1], 0xB504F333);
        X[5] = FIX_MUL32(X[5], 0xB504F333);

        alpha = X[4], X[4] = X[7], X[7] = alpha;
        alpha = X[5], X[5] = X[6], X[6] = alpha;

        alpha = X[1] - X[6], beta = X[1] + X[6];
        X[1] = alpha, X[6] = beta;

        alpha = X[2] - X[4], beta = X[2] + X[4];
        X[2] = alpha, X[4] = beta;

        alpha = X[3] - X[5], beta = X[3] + X[5];
        X[3] = alpha, X[5] = beta;
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
    if(N_bits == 3) {
        register FIX alpha, beta;

        alpha = X[0] + X[7], beta = X[0] - X[7];
        X[0] = alpha, X[7] = beta;

        alpha = X[1] + X[6], beta = X[1] - X[6];
        X[1] = alpha, X[6] = beta;

        alpha = X[2] + X[5], beta = X[2] - X[5];
        X[2] = alpha, X[5] = beta;

        alpha = X[3] + X[4], beta = X[3] - X[4];
        X[3] = alpha, X[4] = beta;

        alpha = X[4], X[4] = X[7], X[7] = alpha;
        alpha = X[5], X[5] = X[6], X[6] = alpha;

        alpha = X[0] + X[3], beta = X[0] - X[3];
        X[0] = alpha, X[3] = beta;

        alpha = X[1] + X[2], beta = X[1] - X[2];
        X[1] = alpha, X[2] = beta;

        alpha = FIX_MUL32(X[4], cosine_table1_1024[1][0]) + FIX_MUL32(X[7], sine_table1_1024[1][0]);
        beta = FIX_MUL32(X[4], sine_table1_1024[1][0]) - FIX_MUL32(X[7], cosine_table1_1024[1][0]);
        X[4] = alpha, X[7] = beta;

        alpha = FIX_MUL32(X[5], cosine_table1_1024[1][1]) + FIX_MUL32(X[6], sine_table1_1024[1][1]);
        beta = FIX_MUL32(X[5], sine_table1_1024[1][1]) - FIX_MUL32(X[6], cosine_table1_1024[1][1]);
        X[5] = alpha, X[6] = -beta;

        alpha = X[2], X[2] = X[3], X[3] = alpha;
        alpha = X[6], X[6] = X[7], X[7] = alpha;

        alpha = X[0] + X[1], beta = X[0] - X[1];
        X[0] = alpha, X[1] = beta;

        alpha = FIX_MUL32(X[2], cosine_table1_1024[0][0]) + FIX_MUL32(X[3], sine_table1_1024[0][0]);
        beta = FIX_MUL32(X[2], sine_table1_1024[0][0]) - FIX_MUL32(X[3], cosine_table1_1024[0][0]);
        X[2] = alpha, X[3] = beta;

        alpha = X[4] + X[5], beta = X[4] - X[5];
        X[4] = alpha, X[5] = beta;

        alpha = X[6] + X[7], beta = X[6] - X[7];
        X[6] = alpha, X[7] = beta;

        X[1] = FIX_MUL32(X[1], 0xB504F333);
        X[5] = FIX_MUL32(X[5], 0xB504F333);
        X[7] = FIX_MUL32(X[7], 0xB504F333);

        alpha = X[6], X[6] = X[7], X[7] = alpha;

        alpha = X[5] - X[6], beta = X[5] + X[6];
        X[5] = alpha, X[6] = beta;
    } else {
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

