#include "decoder.h"

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
