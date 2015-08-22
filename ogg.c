#include "decoder.h"

static uint8_t stream_structure_version;
static uint8_t header_type_flag;
static uint64_t absolute_granule_position;
static uint32_t stream_serial_number;
static uint32_t page_sequence_number;
static uint32_t page_checksum;
static uint8_t page_segments = 0;
static uint8_t segment_size[255];

static uint64_t buffer = 0;
static int bit_position = 0, byte_position = 0;
static int segment_position = 0;
static volatile bool EOP_flag = 0;

static int packet_size_count = 0;
static int total_bytes_read = 0;

uint32_t prefetch_buffer[PREFETCH_BUFFER_SIZE];
unsigned int prefetch_depth = 0, prefetch_position = 0;

uint8_t fetch_byte_from_packet(void)
{
    while(byte_position >= segment_size[segment_position]) {
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

uint32_t read_unsigned_value(int n)
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

FIX read_float32(void)
{
    /*float mantissa = read_unsigned_value(21);
    int exponent = read_unsigned_value(10) - 788;
    if(read_unsigned_value(1)) {
        return -ldexpf(mantissa, exponent);
    }
    return ldexpf(mantissa, exponent);*/
    int32_t mantissa = read_unsigned_value(21);
    int exponent = read_unsigned_value(10) - 788;
    if(read_unsigned_value(1)) {
        printf("-%u * 2 ^ %d\n", mantissa, exponent);
        return -(mantissa >> (-exponent));
    } else {
        printf("%u * 2 ^ %d\n", mantissa, exponent);
        return mantissa >> (-exponent);
    }
}

void prefetch_packet(int offset)
{
    uint8_t *dest = (uint8_t *)prefetch_buffer;
    memset(prefetch_buffer, 0, 4 * PREFETCH_BUFFER_SIZE);
    dest[0] = buffer << offset;
    for(prefetch_depth = 1; prefetch_depth < 4 * PREFETCH_BUFFER_SIZE; prefetch_depth++) {
        dest[prefetch_depth] = fetch_byte_from_packet();
        if(EOP_flag) {
            prefetch_depth = prefetch_depth * 8 + 8;
            prefetch_position = offset;
            return;
        }
    }
    ERROR(ERROR_OGG, "Prefetch target is too large.\n");
}

void fetch_page(void)
{
    if(read_unsigned_byte() != 'O' ||
       read_unsigned_byte() != 'g' ||
       read_unsigned_byte() != 'g' ||
       read_unsigned_byte() != 'S') {
        fprintf(stderr, "Missing Ogg page header signature\n");
        return;
    }

    stream_structure_version = read_unsigned_byte();

    if(stream_structure_version != 0x00) {
        fprintf(stderr, "Unsupported Ogg stream version: %u\n", stream_structure_version);
        return;
    }

    header_type_flag = read_unsigned_byte();

    absolute_granule_position = read_unsigned_long_long();

    stream_serial_number = read_unsigned_long();

    page_sequence_number = read_unsigned_long();

    page_checksum = read_unsigned_long();

    page_segments = read_unsigned_byte();

    for(int i = 0; i < page_segments; i++) {
        segment_size[i] = read_unsigned_byte();
    }

    segment_position = 0;
    byte_position = 0;
}

static void close_packet(void)
{
    int remainder = 0;

    while(1) {
        for(; byte_position < segment_size[segment_position]; byte_position++) {
            read_unsigned_byte();
            packet_size_count++;
            remainder++;
        }
        if(segment_size[segment_position] == 255) {
            if(++segment_position >= page_segments) {
                fetch_page();
            }
            byte_position = 0;
        } else {
            break;
        }
    }
    if(remainder) {
        WARNING("Last %d bytes skipped during packet decode.\n", remainder);
    }
}

static int open_packet(void)
{
    if(++segment_position >= page_segments) {
        fetch_page();
        if(EOF_flag) return 1;
    }
    byte_position = 0;
    bit_position = 0;
    packet_size_count = 0;
    prefetch_depth = 0;
    EOP_flag = false;
    return 0;
}

void decode(void)
{
    while(1) {
        if(open_packet()) break;

        decode_packet();

        close_packet();
    }
    INFO("%d bytes read from the logical stream.\n", total_bytes_read);
}
