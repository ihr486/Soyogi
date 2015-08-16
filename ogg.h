#ifndef OGG_H
#define OGG_H

#define MASK32(n) (((uint64_t)1 << (n)) - 1)

extern uint8_t page_segments;
extern uint8_t segment_size[255];

extern uint64_t buffer;
extern int bit_position, byte_position;
extern int segment_position;
extern bool EOP_flag;

extern int packet_size_count;
extern int total_bytes_read;

void fetch_page(void);

inline uint8_t fetch_byte_from_packet(void)
{
    if(byte_position >= segment_size[segment_position]) {
        if(segment_size[segment_position] < 255) {
            EOP_flag = true;
            return 0;
        } else {
            if(++segment_position >= page_segments) {
                fetch_page();
            } else {
                byte_position = 0;
            }
        }
    }
    byte_position++;
    packet_size_count++;
    total_bytes_read++;

    return read_unsigned_byte();
}

inline uint32_t read_unsigned_value(int n)
{
    uint64_t ret = buffer & MASK32(bit_position);

    int pos = bit_position;

    while(pos < n) {
        uint8_t b = fetch_byte_from_packet();

        ret |= ((uint64_t)b << pos);

        pos += 8;
    }

    buffer = ret >> n;
    bit_position = pos - n;

    return ret & MASK32(n);
}

inline float read_float32(void)
{
    float mantissa = read_unsigned_value(21);
    int exponent = read_unsigned_value(10) - 788;
    if(read_unsigned_value(1)) {
        return -ldexpf(mantissa, exponent);
    }
    return ldexpf(mantissa, exponent);
}

#endif
