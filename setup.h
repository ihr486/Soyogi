#ifndef SETUP_H
#define SETUP_h

#define SETUP_STACK_SIZE 20480

extern uint8_t setup_stack[SETUP_STACK_SIZE];
extern uint16_t setup_stack_head;

inline uint16_t setup_get_head(void)
{
    return setup_stack_head;
}

inline uint8_t setup_get_byte(uint16_t p)
{
    return setup_stack[p];
}

inline void setup_set_byte(uint16_t p, uint8_t val)
{
    setup_stack[p] = val;
}

inline uint16_t setup_get_short(uint16_t p)
{
    return setup_stack[p] | (setup_stack[p + 1] << 8);
}

inline void setup_set_short(uint16_t p, uint16_t val)
{
    setup_stack[p] = val;
    setup_stack[p + 1] = val >> 8;
}

inline void setup_push_short(uint16_t val)
{
    setup_stack[setup_stack_head] = val;
    setup_stack[setup_stack_head + 1] = val >> 8;
}

inline uint16_t setup_allocate_natural(uint16_t size)
{
    uint16_t ret = ((setup_stack_head + 3) / 4) * 4;

    if(ret + size >= SETUP_STACK_SIZE)
        ERROR(ERROR_SETUP, "Setup stack overflowed.\n");

    setup_stack_head = ret + size;
    return ret;
}

inline uint16_t setup_allocate_packed(uint16_t size)
{
    uint16_t ret = setup_stack_head;

    if(ret + size >= SETUP_STACK_SIZE)
        ERROR(ERROR_SETUP, "Setup stack overflowed.\n");

    setup_stack_head = ret + size;
    return ret;
}

inline void *setup_ref(uint16_t p)
{
    return setup_stack + p;
}

#endif
