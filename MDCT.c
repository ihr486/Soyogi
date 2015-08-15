#include "decoder.h"

static const float PI = 3.141592653589793f;

static inline int reverse_bits(int n, int l)
{
    int ret = 0;
    while(l >>= 1) {
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

static void FDCT_R_IV(float *X, int n);

static void FDCT_R_III(float *X, int n)
{
    if(n > 1) {
        FDCT_R_III(X, n / 2);
        FDCT_R_IV(X + (n / 2), n / 2);

        for(int i = 0; i < n / 2; i++) {
            float alpha = X[i] + X[i + n / 2];
            float beta = X[i] - X[i + n / 2];

            X[i] = alpha;
            X[i + n / 2] = beta;
        }
        for(int i = 0; i < n / 4; i++) {
            swap(&X[i + n / 2], &X[n - 1 - i]);
        }
    }
}

static void FDCT_R_IV(float *X, int n)
{
    if(n == 1) {
        X[0] *= cos(PI * 0.25);
    } else {
        FDCT_R_III(X, n / 2);
        FDCT_R_IV(X + (n / 2), n / 2);

        for(int i = 0; i < n / 2; i++) {
            float alpha = X[i] + X[i + n / 2];
            float beta = X[i] - X[i + n / 2];

            X[i] = alpha * 0.5 / cos(PI * 0.5 / n * (i + 0.5));
            X[i + n / 2] = beta * 0.5 / sin(PI * 0.5 / n * (i + 0.5));
        }
        for(int i = 0; i < n / 4; i++) {
            swap(&X[i + n / 2], &X[n - 1 - i]);
        }
    }
}

void FDCT_IV(float *X, int n)
{
    for(int i = n - 1; i > 0; i--) {
        X[i] += X[i - 1];
    }
    for(int i = 2; i < n; i <<= 1) {
        for(int j = 0; j < i / 2; j++) {
            for(int k = 0; k < (n / i - 1); k++) {
                X[n - 1 - k * i - j] += X[n - 1 - ((k + 1) * i) - j];
            }
        }
    }
    for(int i = 0; i < n; i++) {
        int j = reverse_bits(i, n);
        if(i < j) {
            swap(&X[i], &X[j]);
        }
    }

    FDCT_R_IV(X, n);

    for(int i = 0; i < n; i++) {
        //X[i] *= n;
    }
}

void DCT_IV(const float *X, float *Y, int n)
{
    for(int i = 0; i < n; i++) {
        Y[i] = 0;
        for(int j = 0; j < n; j++) {
            Y[i] += X[j] * cos(PI / n * (i + 0.5) * (j + 0.5));
        }
    }
}

void slow_DCT_IV(float *X, int n)
{
    float Y[n];

    DCT_IV(X, Y, n);

    memcpy(X, Y, sizeof(Y));
}
