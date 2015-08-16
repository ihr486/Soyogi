#include "decoder.h"

static uint8_t stream_structure_version;
static uint8_t header_type_flag;
static uint64_t absolute_granule_position;
static uint32_t stream_serial_number;
static uint32_t page_sequence_number;
static uint32_t page_checksum;
uint8_t page_segments = 0;
uint8_t segment_size[255];

uint64_t buffer = 0;
int bit_position = 0, byte_position = 0;
int segment_position = 0;
bool EOP_flag = 0;

int packet_size_count = 0;
int total_bytes_read = 0;

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
    if(remainder)
        WARNING("Last %d bytes skipped during packet decode.\n", remainder);
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
    INFO("%d bytes read from the logical stream.\n", total_bytes_read);
}
