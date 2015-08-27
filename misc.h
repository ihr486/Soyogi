#ifndef MISCELLANEOUS_H
#define MISCELLANEOUS_H

#define STR(x) STR2(x)
#define STR2(x) #x

#ifdef DEBUG
#define ERROR(code, ...) error_verbose(code, __VA_ARGS__)
#define INFO(...) printf(__VA_ARGS__)
#define WARNING(...) fprintf(stderr, __VA_ARGS__)
#else
#define ERROR(code, ...) error_silent(code)
#define INFO(...) {}
#define WARNING(...) {}
#endif

void __attribute__((noreturn)) error_verbose(int code, const char *fmt, ...);
void __attribute__((noreturn)) error_silent(int code);

extern jmp_buf jump_env;

#define ERROR_CODEBOOK 1
#define ERROR_SETUP 2
#define ERROR_VQ 3
#define ERROR_FLOOR 4
#define ERROR_FLOOR1 5
#define ERROR_RESIDUE 6
#define ERROR_MAPPING 7
#define ERROR_MODE 8
#define ERROR_OGG 9
#define ERROR_VORBIS 10
#define DECODE_FINISHED 11

#ifdef FIXED_POINT
typedef int32_t DATA_TYPE;
typedef uint32_t COEFF_TYPE;

#define MUL(x, y) (((int64_t)(x) * (int64_t)(y)) >> 32)
#else
typedef float DATA_TYPE;
typedef float COEFF_TYPE;

#define MUL(x, y) ((x) * (y))
#endif

#define MS_ELAPSED(t) ((double)(clock() - t) / (double)CLOCKS_PER_SEC * 1000.0)

#endif
