#include "decoder.h"

#ifdef __CYGWIN__
#include <windows.h>
#endif

jmp_buf jump_env;

void __attribute__((noreturn)) error_verbose(int code, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fflush(stdout);

    vfprintf(stderr, fmt, ap);

    va_end(ap);

    longjmp(jump_env, code);
}

void __attribute__((noreturn)) error_silent(int code)
{
    longjmp(jump_env, code);
}

double get_us(void)
{
#ifdef __CYGWIN__
    LARGE_INTEGER freq, now;

    if(!QueryPerformanceFrequency(&freq)) return 0.0;

    if(!QueryPerformanceCounter(&now)) return 0.0;

    return (double)now.QuadPart / (double)freq.QuadPart * 1E+6;
#else
    return (double)clock() / (double)CLOCKS_PER_SEC * 1E+6;
#endif
}

double get_counter_freq(void)
{
#ifdef __CYGWIN__
    LARGE_INTEGER freq;

    if(!QueryPerformanceFrequency(&freq)) return 0.0;

    return (double)freq.QuadPart;
#else
    return (double)CLOCKS_PER_SEC;
#endif
}
