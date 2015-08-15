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
volatile bool EOP_flag = 0;

static int packet_size_count = 0;
static int total_bytes_read = 0;

#define MASK32(n) (((uint64_t)1 << (n)) - 1)

static void fetch_page(void)
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

static uint8_t fetch_byte_from_packet(void)
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

void push_bits(uint32_t val, int n)
{
    buffer = (buffer << n) | val;
}

int32_t read_signed_value(int n)
{
    uint32_t ret = read_unsigned_value(n);

    return (int32_t)(ret << (32 - n)) >> (32 - n);
}

float read_float32(void)
{
    float mantissa = read_unsigned_value(21);
    int exponent = read_unsigned_value(10) - 788;
    if(read_unsigned_value(1)) {
        return -ldexpf(mantissa, exponent);
    }
    return ldexpf(mantissa, exponent);
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
    if(remainder)
        INFO("%d/%d bytes read\n", packet_size_count - remainder, packet_size_count);
    //printf("Packet size = %d.\n", packet_size_count);
}

static int open_packet(void)
{
    if(++segment_position >= page_segments) {
        fetch_page();
        if(reached_EOF()) return 1;
    }
    byte_position = 0;
    bit_position = 0;
    packet_size_count = 0;
    EOP_flag = false;
    return 0;
}

void decode(void)
{
    while(1) {
        if(open_packet()) break;

        if(decode_packet()) break;

        close_packet();
    }
    printf("%d bytes read.\n", total_bytes_read);
}
