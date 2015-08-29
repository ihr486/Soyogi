#include "decoder.h"

inline void swap(DATA_TYPE *x, DATA_TYPE *y)
{
    DATA_TYPE t = *x;
    *x = *y;
    *y = t;
}

void FDCT_R_II(DATA_TYPE *X, int N_bits);

void FDCT_R_IV(DATA_TYPE *X, int N_bits)
{
    if(N_bits == 3) {
        register DATA_TYPE temp;

//N=8 DCT-IV butterfly
        temp = MUL(X[0], cosine_table0_1024[3][0]) + MUL(X[7], sine_table0_1024[3][0]);
        X[7] = MUL(X[0], sine_table0_1024[3][0]) - MUL(X[7], cosine_table0_1024[3][0]);
        X[0] = temp;

        temp = MUL(X[1], cosine_table0_1024[3][1]) + MUL(X[6], sine_table0_1024[3][1]);
        X[6] = -(MUL(X[1], sine_table0_1024[3][1]) - MUL(X[6], cosine_table0_1024[3][1]));
        X[1] = temp;

        temp = MUL(X[2], cosine_table0_1024[3][2]) + MUL(X[5], sine_table0_1024[3][2]);
        X[5] = MUL(X[2], sine_table0_1024[3][2]) - MUL(X[5], cosine_table0_1024[3][2]);
        X[2] = temp;

        temp = MUL(X[3], cosine_table0_1024[3][3]) + MUL(X[4], sine_table0_1024[3][3]);
        X[4] = -(MUL(X[3], sine_table0_1024[3][3]) - MUL(X[4], cosine_table0_1024[3][3]));
        X[3] = temp;

//N=4 DCT-II butterfly
        temp = X[0] - X[3];
        X[0] += X[3];
        X[3] = temp;

        temp = X[1] - X[2];
        X[1] += X[2];
        X[2] = temp;

//N=4 DCT-II butterfly
        temp = X[7] - X[4];
        X[4] += X[7];
        X[7] = temp;

        temp = X[6] - X[5];
        X[5] += X[6];
        X[6] = temp;

//N=2 DCT-II butterfly
        temp = X[0] - X[1];
        X[0] += X[1];
        X[1] = temp;

//N=2 DCT-IV butterfly
        temp = MUL(X[3], cosine_table0_1024[1][0]) + MUL(X[2], sine_table0_1024[1][0]);
        X[3] = MUL(X[3], sine_table0_1024[1][0]) - MUL(X[2], cosine_table0_1024[1][0]);
        X[2] = temp;

//N=2 DCT-II butterfly
        temp = X[4] - X[5];
        X[4] += X[5];
        X[5] = temp;

//N=2 DCT-IV butterfly
        temp = MUL(X[7], cosine_table0_1024[1][0]) + MUL(X[6], sine_table0_1024[1][0]);
        X[7] = MUL(X[7], sine_table0_1024[1][0]) - MUL(X[6], cosine_table0_1024[1][0]);
        X[6] = temp;

//N=1 DCT-IV butterflies
        X[1] = MUL(X[1], cosine_table0_1024[0][0]);
        X[5] = MUL(X[5], cosine_table0_1024[0][0]);

//N=8 DCT-IV post-process
        temp = X[4], X[4] = X[7], X[7] = temp;
        temp = X[5], X[5] = X[6], X[6] = temp;

        temp = X[1] - X[6];
        X[6] += X[1];
        X[1] = temp;

        temp = X[2] - X[4];
        X[4] += X[2];
        X[2] = temp;

        temp = X[3] - X[5];
        X[5] += X[3];
        X[3] = temp;
    } else {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            UNSIGNED_COEFF_TYPE c = cosine_table0_1024[N_bits][i];
            UNSIGNED_COEFF_TYPE s = sine_table0_1024[N_bits][i];

            DATA_TYPE alpha = MUL(X[i], c) + MUL(X[N - 1 - i], s);
            DATA_TYPE beta = MUL(X[i], s) - MUL(X[N - 1 - i], c);

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

            DATA_TYPE alpha = X[i] - X[r];
            DATA_TYPE beta = X[i] + X[r];

            X[i] = alpha;
            X[r] = beta;
        }
    }
}

void FDCT_R_II(DATA_TYPE *X, int N_bits)
{
    if(N_bits == 3) {
        register DATA_TYPE temp;

//N=8 DCT-II butterfly
        temp = X[0] - X[7];
        X[0] += X[7];
        X[7] = temp;

        temp = X[1] - X[6];
        X[1] += X[6];
        X[6] = temp;

        temp = X[2] - X[5];
        X[2] += X[5];
        X[5] = temp;

        temp = X[3] - X[4];
        X[3] += X[4];
        X[4] = temp;

//N=4 DCT-II butterfly
        temp = X[0] - X[3];
        X[0] += X[3];
        X[3] = temp;

        temp = X[1] - X[2];
        X[1] += X[2];
        X[2] = temp;

//N=4 DCT-IV butterfly
        temp = MUL(X[7], cosine_table0_1024[2][0]) + MUL(X[4], sine_table0_1024[2][0]);
        X[7] = MUL(X[7], sine_table0_1024[2][0]) - MUL(X[4], cosine_table0_1024[2][0]);
        X[4] = temp;

        temp = MUL(X[6], cosine_table0_1024[2][1]) + MUL(X[5], sine_table0_1024[2][1]);
        X[6] = -(MUL(X[6], sine_table0_1024[2][1]) - MUL(X[5], cosine_table0_1024[2][1]));
        X[5] = temp;

//N=2 DCT-II butterfly
        temp = X[0] - X[1];
        X[0] += X[1];
        X[1] = temp;

//N=2 DCT-IV butterfly
        temp = MUL(X[3], cosine_table0_1024[1][0]) + MUL(X[2], sine_table0_1024[1][0]);
        X[3] = MUL(X[3], sine_table0_1024[1][0]) - MUL(X[2], cosine_table0_1024[1][0]);
        X[2] = temp;

//N=2 DCT-II butterfly
        temp = X[4] - X[5];
        X[4] += X[5];
        X[5] = temp;

//N=2 DCT-II butterfly
        temp = X[7] - X[6];
        X[6] += X[7];
        X[7] = temp;

//N=1 DCT-IV butterflies
        X[1] = MUL(X[1], cosine_table0_1024[0][0]);
        X[5] = MUL(X[5], cosine_table0_1024[0][0]);
        X[7] = MUL(X[7], cosine_table0_1024[0][0]);

//N=4 DCT-IV post-process
        temp = X[6], X[6] = X[7], X[7] = temp;

        temp = X[5] - X[6];
        X[6] += X[5];
        X[5] = temp;
    } else {
        int N = 1 << N_bits;

        for(int i = 0; i < N / 2; i++) {
            DATA_TYPE alpha = X[i] + X[N - 1 - i];
            DATA_TYPE beta = X[i] - X[N - 1 - i];

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

void FDCT_IV(DATA_TYPE *X, int N_bits)
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

/*const float PI = 3.141592653589793f;

void FDCT_IV(DATA_TYPE *X, int N_bits)
{
    int N = 1 << N_bits;
    DATA_TYPE Y[N];

    for(int i = 0; i < N; i++) {
        Y[i] = 0;
        for(int j = 0; j < N; j++) {
            Y[i] += X[j] * cos(PI / N * (i + 0.5f) * (j + 0.5f));
        }
    }

    memcpy(X, Y, sizeof(DATA_TYPE) * N);
}*/
