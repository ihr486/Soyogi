#ifndef OGG_H
#define OGG_H

#define PREFETCH_BUFFER_SIZE (512)

#define MASK32(n) (((uint64_t)1 << (n)) - 1)

extern uint32_t prefetch_buffer[PREFETCH_BUFFER_SIZE];
extern unsigned int prefetch_depth, prefetch_position;

void fetch_page(void);
uint32_t read_unsigned_value(int n);
DATA_TYPE read_float32(void);
void prefetch_packet(int offset);

/*inline uint32_t read_unsigned_value_PF(int n)
{
    uint32_t ret = (prefetch_buffer[prefetch_position >> 5] >> (prefetch_position & 31)) | (prefetch_buffer[(prefetch_position >> 5) + 1] << (32 - (prefetch_position & 31)));
    prefetch_position += n;
    return ret & MASK32(n);
}*/

inline uint32_t read_bit_PF(void)
{
    uint32_t ret = prefetch_buffer[prefetch_position >> 5] >> (prefetch_position & 31);
    if(prefetch_position++ > prefetch_depth) {
        printf("Prefetch buffer overrun.\n");
    }
    return ret & 1;
}

inline uint32_t read_unsigned_value_PF(int n)
{
    uint32_t ret = 0, mask = 1;
    for(int i = 0; i < n; i++) {
        if(read_bit_PF()) {
            ret |= mask;
        }
        mask <<= 1;
    }
    return ret;
}

inline bool prefetch_buffer_exhausted(void)
{
    return prefetch_position >= prefetch_depth;
}

#endif
