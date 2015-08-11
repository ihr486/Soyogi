#include "decoder.h"

uint8_t setup_stack[SETUP_STACK_SIZE];
uint16_t setup_stack_head = 0;

/*uint16_t setup_push_unsigned_short(uint16_t val)
{
    if(setup_stack_head > SETUP_STACK_SIZE - sizeof(uint16_t))
        ERROR("Setup stack overflowed.\n");
    *(uint16_t *)(setup_stack + setup_stack_head) = val;
    setup_stack_head += sizeof(uint16_t);
}

void setup_push_float(float val)
{
    if(setup_stack_head > SETUP_STACK_SIZE - sizeof(float))
        ERROR("Setup stack overflowed.\n");
    *(float *)(setup_stack + setup_stack_head) = val;
    setup_stack_head += sizeof(float);
}

void setup_push_unsigned_long(uint32_t val)
{
    if(setup_stack_head > SETUP_STACK_SIZE - sizeof(uint32_t))
        ERROR("Setup stack overflowed.\n");
    *(uint32_t *)(setup_stack + setup_stack_head) = val;
    setup_stack_head += sizeof(uint32_t);
}

void setup_push_block(void *p, size_t len)
{
    if(setup_stack_head > SETUP_STACK_SIZE - len)
        ERROR("Setup stack overflowed,\n");
    memcpy(setup_stack + setup_stack_head, p, len);
    setup_stack_head += len;
}

uint16_t *setup_ref_short(uint16_t p)
{
    return (uint16_t *)(setup_stack + p);
}

int16_t setup_get_head_offset(uint16_t p)
{
    return setup_stack_head - p;
}

uint16_t setup_get_unsigned_short(uint16_t p)
{
    return *(uint16_t *)(setup_stack + p);
}

float setup_get_float(uint16_t p)
{
    return *(float *)(setup_stack + p);
}

uint32_t setup_get_unsigned_long(uint16_t p)
{
    return *(uint32_t *)(setup_stack + p);
}

uint16_t setup_allocate_short(uint16_t n)
{
    if(setup_stack_head > SETUP_STACK_SIZE - n * sizeof(uint16_t))
        ERROR("Setup stack overflowed.\n");

    uint16_t ret = setup_stack_head;
    setup_stack_head += sizeof(uint16_t) * n;
    return ret;
}*/
