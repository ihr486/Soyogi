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
#define INFO(...)
#define WARNING(...)
#endif

void __attribute__((noreturn)) error_verbose(int code, const char *fmt, ...);
void __attribute__((noreturn)) error_silent(int code);

extern jmp_buf jump_env;

#define ERROR_CODEBOOK 1
#define ERROR_SETUP 2
#define ERROR_VQ 3
#define ERROR_FLOOR 4
#define ERROR_FLOOR1 5

#endif
