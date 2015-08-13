#include "decoder.h"

typedef enum {
    LOOKUP_NONE = 0,
    LOOKUP_TYPE1_BYTE,
    LOOKUP_TYPE1_SHORT,
    LOOKUP_TYPE2_BYTE,
    LOOKUP_TYPE2_SHORT
} lookup_mode_t;

typedef struct codebook_tag {
    uint16_t entries;
    uint8_t dimension;
    uint8_t lookup_type;
    uint16_t huffman;
    uint16_t lookup;
} codebook_t;

typedef struct VQ_header_tag {
    float minimum_value;
    float delta_value;
    uint8_t sequence_p;
    uint8_t lookup_mode;
    uint16_t lookup_values;
    uint16_t table;
} VQ_header_t;

static int codebook_num = 0;
static codebook_t *codebook_list = NULL;

static void decode_codebook(int index);

void setup_codebooks(void)
{
    codebook_num = read_unsigned_value(8) + 1;
    codebook_list = setup_ref(setup_allocate_natural(sizeof(codebook_t) * codebook_num));

    for(int i = 0; i < codebook_num; i++) {
        decode_codebook(i);
    }
}

static inline void insert_codeword(const codebook_t *cb, uint16_t index, uint8_t length, uint32_t level_depth[32])
{
    uint32_t codeword = level_depth[length - 1];

    int j;
    for(j = length - 2; j >= 0; j--) {
        if((level_depth[j] * 2 - codeword) & 1) {
            break;
        } else {
            codeword = level_depth[j];
        }
    }
    for(++j; j < length - 1; j++) {
        level_depth[j] = codeword + 1;
        codeword <<= 1;
    }
    level_depth[length - 1] = codeword + 1;
    codeword <<= 32 - length;

    //printf("\tCodeword[%d]:%d = %#X\n", index, length, codeword);
    //printf("\t");

    uint16_t pos = cb->huffman;

    if(cb->entries < 128) {
        if(cb->huffman == setup_get_head()) {
            pos = setup_allocate_packed(2);
            setup_set_byte(pos, 0);
            setup_set_byte(pos + 1, 0);
        }
        pos += (codeword >> 31) & 1;
        //printf("%d->", pos);
        for(int i = 1; i < length; i++) {
            if(!setup_get_byte(pos)) {
                uint16_t new_pos = setup_allocate_packed(2);
                setup_set_byte(pos, (new_pos - cb->huffman) / 2);
                setup_set_byte(new_pos, 0);
                setup_set_byte(new_pos + 1, 0);
                pos = new_pos;
            } else {
                pos = cb->huffman + setup_get_byte(pos) * 2;
            }
            pos += (codeword >> (31 - i)) & 1;
            //printf("%d->", pos);
        }
        setup_set_byte(pos, 0x80 | index);
        //printf("\n");
    } else if(cb->entries < 2048) {
        if(cb->huffman == setup_get_head()) {
            pos = setup_allocate_packed(3);
            setup_set_byte(pos, 0);
            setup_set_byte(pos + 1, 0);
            setup_set_byte(pos + 2, 0);
        }
        //printf("%d->", pos);
        for(int i = 0; i < length - 1; i++) {
            if((codeword >> (31 - i)) & 1) {
                uint16_t next = ((uint16_t)setup_get_byte(pos + 2) << 4) | (setup_get_byte(pos + 1) >> 4);
                if(!next) {
                    uint16_t new_pos = setup_allocate_packed(3);
                    setup_set_byte(pos + 1, setup_get_byte(pos + 1) | (((new_pos - cb->huffman) / 3) << 4));
                    setup_set_byte(pos + 2, ((new_pos - cb->huffman) / 3) >> 4);
                    setup_set_byte(new_pos, 0);
                    setup_set_byte(new_pos + 1, 0);
                    setup_set_byte(new_pos + 2, 0);
                    pos = new_pos;
                } else {
                    pos = cb->huffman + next * 3;
                }
            } else {
                uint16_t next = setup_get_byte(pos) | ((uint16_t)(setup_get_byte(pos + 1) & 0x0F) << 8);
                if(!next) {
                    uint16_t new_pos = setup_allocate_packed(3);
                    setup_set_byte(pos + 1, setup_get_byte(pos + 1) | (((new_pos - cb->huffman) / 3) >> 8));
                    setup_set_byte(pos, (new_pos - cb->huffman) / 3);
                    setup_set_byte(new_pos, 0);
                    setup_set_byte(new_pos + 1, 0);
                    setup_set_byte(new_pos + 2, 0);
                    pos = new_pos;
                } else {
                    pos = cb->huffman + next * 3;
                }
            }
            //printf("%d->", pos);
        }
        if((codeword >> (32 - length)) & 1) {
            setup_set_byte(pos + 1, setup_get_byte(pos + 1) | ((uint8_t)index << 4));
            setup_set_byte(pos + 2, 0x80 | (index >> 4));
        } else {
            setup_set_byte(pos, index);
            setup_set_byte(pos + 1, setup_get_byte(pos + 1) | 0x08 | (index >> 8));
        }
        //printf("\n");
    } else if(cb->entries < 32768) {
        if(cb->huffman == setup_get_head()) {
            pos = setup_allocate_packed(4);
            setup_set_byte(pos, 0);
            setup_set_byte(pos + 1, 0);
            setup_set_byte(pos + 2, 0);
            setup_set_byte(pos + 3, 0);
        }
        pos += 2 * ((codeword >> 31) & 1);
        for(int i = 1; i < length; i++) {
            uint16_t next = setup_get_byte(pos) | (setup_get_byte(pos + 1) << 8);
            if(!next) {
                uint16_t new_pos = setup_allocate_packed(4);
                setup_set_byte(pos, (new_pos - cb->huffman) / 4);
                setup_set_byte(pos + 1, ((new_pos - cb->huffman) / 4) >> 8);
                setup_set_byte(new_pos, 0);
                setup_set_byte(new_pos + 1, 0);
                setup_set_byte(new_pos + 2, 0);
                setup_set_byte(new_pos + 3, 0);
                pos = new_pos;
            } else {
                pos = cb->huffman + next * 4;
            }
            pos += 2 * ((codeword >> (31 - i)) & 1);
        }
        setup_set_byte(pos, index);
        setup_set_byte(pos + 1, 0x80 | (index >> 8));
    } else {
        ERROR(ERROR_CODEBOOK, "Too many entries in a codebook.\n");
    }
}

static void decode_codebook(int index)
{
    uint32_t sync_pattern = read_unsigned_value(24);
    if(sync_pattern != 0x564342)
        ERROR(ERROR_CODEBOOK, "Corrupted codebook signature.\n");

    codebook_t *cb = codebook_list + index;

    cb->dimension = read_unsigned_value(16);
    cb->entries = read_unsigned_value(24);
    cb->lookup_type = LOOKUP_NONE;
    cb->huffman = setup_get_head();

    INFO("Codebook[%d]: %dD * %d entries\n", index, cb->dimension, cb->entries);

    uint32_t level_depth[32];

    memset(level_depth, 0, sizeof(level_depth));

    uint8_t ordered_flag = read_unsigned_value(1);

    if(ordered_flag) {
        uint32_t current_entry = 0;
        uint8_t current_length = read_unsigned_value(5) + 1;

        while(current_entry < cb->entries) {
            uint32_t number = read_unsigned_value(ilog(cb->entries - current_entry));

            for(unsigned int i = 0; i < number; i++) {
                insert_codeword(cb, i, current_length, level_depth);
            }

            current_length++;
            current_entry += number;
        }
    } else {
        uint8_t sparse_flag = read_unsigned_value(1);

        if(sparse_flag) {
            for(int i = 0; i < cb->entries; i++) {
                uint8_t active_flag = read_unsigned_value(1);

                if(active_flag) {
                    uint8_t length = read_unsigned_value(5) + 1;

                    insert_codeword(cb, i, length, level_depth);
                }
            }
        } else {
            for(int i = 0; i < cb->entries; i++) {
                uint8_t length = read_unsigned_value(5) + 1;

                insert_codeword(cb, i, length, level_depth);
            }
        }
    }

    cb->lookup_type = read_unsigned_value(4);

    if(cb->lookup_type > 0) {
        printf("\tVQ lookup type %d.\n", cb->lookup_type);

        cb->lookup = setup_allocate_natural(sizeof(VQ_header_t));

        VQ_header_t *header = setup_ref(cb->lookup);

        header->minimum_value = read_float32();
        header->delta_value = read_float32();

        uint8_t value_bits = read_unsigned_value(4) + 1;
        header->sequence_p = read_unsigned_value(1);

        if(cb->lookup_type == 1) {
            header->lookup_values = lookup1_values(cb->dimension, cb->entries);
        } else if(cb->lookup_type == 2) {
            header->lookup_values = cb->dimension * cb->entries;
        } else {
            ERROR(ERROR_VQ, "Unsupported VQ type %d.\n", cb->lookup_type);
        }

        INFO("\t%d bits * %d lookup values\n", value_bits, header->lookup_values);

        if(value_bits <= 8) {
            header->lookup_mode = 8;
            header->table = setup_allocate_packed(header->lookup_values);

            uint8_t *table = setup_ref(header->table);

            for(int i = 0; i < header->lookup_values; i++) {
                table[i] = read_unsigned_value(value_bits);
            }
        } else {
            header->lookup_mode = 16;
            header->table = setup_allocate_natural(header->lookup_values * 2);

            uint16_t *table = setup_ref(header->table);

            for(int i = 0; i < header->lookup_values; i++) {
                table[i] = read_unsigned_value(value_bits);
            }
        }
    }
}
