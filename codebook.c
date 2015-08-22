#include "decoder.h"

int codebook_num = 0;
codebook_t *codebook_list = NULL;

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

    uint16_t pos = cb->huffman;

    if(cb->entries < 128) {
        if(cb->huffman == setup_get_head()) {
            setup_push_short(0);
        }
        pos += (codeword >> 31) & 1;
        for(int i = 1; i < length; i++) {
            if(!setup_get_byte(pos)) {
                uint16_t new_pos = setup_get_head();
                setup_set_byte(pos, (new_pos - cb->huffman) / 2);
                setup_push_short(0);
                pos = new_pos;
            } else {
                pos = cb->huffman + setup_get_byte(pos) * 2;
            }
            pos += (codeword >> (31 - i)) & 1;
        }
        setup_set_byte(pos, 0x80 | index);
    } else if(cb->entries < 2048) {
        if(cb->huffman == setup_get_head()) {
            pos = setup_allocate_packed(3);
            setup_set_byte(pos, 0);
            setup_set_byte(pos + 1, 0);
            setup_set_byte(pos + 2, 0);
        }
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
        }
        if((codeword >> (32 - length)) & 1) {
            setup_set_byte(pos + 1, setup_get_byte(pos + 1) | ((uint8_t)index << 4));
            setup_set_byte(pos + 2, 0x80 | (index >> 4));
        } else {
            setup_set_byte(pos, index);
            setup_set_byte(pos + 1, setup_get_byte(pos + 1) | 0x08 | (index >> 8));
        }
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
            uint16_t next = setup_get_byte(pos) | ((uint16_t)setup_get_byte(pos + 1) << 8);
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
    cb->lookup_type = 0;
    cb->huffman = setup_get_head();

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
            int active_codeword_count = 0, last_codeword = 0;
            for(int i = 0; i < cb->entries; i++) {
                uint8_t active_flag = read_unsigned_value(1);

                if(active_flag) {
                    uint8_t length = read_unsigned_value(5) + 1;

                    insert_codeword(cb, i, length, level_depth);

                    active_codeword_count++;
                    last_codeword = i;
                }
            }
            if(active_codeword_count == 1) {
                insert_codeword(cb, last_codeword, 1, level_depth);
            }
        } else {
            for(int i = 0; i < cb->entries; i++) {
                uint8_t length = read_unsigned_value(5) + 1;

                insert_codeword(cb, i, length, level_depth);
            }
            if(cb->entries == 1) {
                insert_codeword(cb, 1, 1, level_depth);
            }
        }
    }

    cb->lookup_type = read_unsigned_value(4);

    if(cb->lookup_type > 0) {
        cb->lookup = setup_allocate_natural(sizeof(VQ_header_t));

        VQ_header_t *header = setup_ref(cb->lookup);

        header->minimum_value = read_float32();
        header->delta_value = read_float32();

        printf("Min = %d, Delta = %d\n", header->minimum_value, header->delta_value);

        uint8_t value_bits = read_unsigned_value(4) + 1;
        header->sequence_p = read_unsigned_value(1);

        if(cb->lookup_type == 1) {
            header->lookup_values = lookup1_values(cb->dimension, cb->entries);
        } else if(cb->lookup_type == 2) {
            header->lookup_values = cb->dimension * cb->entries;
        } else {
            ERROR(ERROR_VQ, "Unsupported VQ type %d.\n", cb->lookup_type);
        }

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

int16_t __attribute__((hot)) lookup_scalar(int index)
{
    codebook_t *cb = &codebook_list[index];

    uint32_t pos = 0;
    int16_t ret = 0;

    if(cb->entries < 128) {
        while(!(pos & 0x80)) {
            pos = setup_get_byte(cb->huffman + pos * 2 + read_bit_PF());
        }
        ret = pos & 0x7F;
    } else if(cb->entries < 2048) {
        while(!(pos & 0x800)) {
            pos = setup_get_long(cb->huffman + pos * 3);
            if(read_bit_PF()) {
                pos >>= 12;
            }
            pos &= 0xFFF;
        }
        ret = pos & 0x7FF;
    } else {
        while(!(pos & 0x8000)) {
            pos = pos * 4 + read_bit_PF() * 2;

            pos = setup_get_short(cb->huffman + pos);
        }
        ret = pos & 0x7FFF;
    }
    return prefetch_buffer_exhausted() ? -1 : ret;
}

int __attribute__((hot)) lookup_vector(FIX *v, int offset, int index, int step, int period)
{
    codebook_t *cb = &codebook_list[index];
    VQ_header_t *vq = setup_ref(cb->lookup);

    int lookup_offset = lookup_scalar(index);

    if(lookup_offset < 0) return 0;

    switch(cb->lookup_type) {
    case 1: {
        FIX last = 0;
        int index_divisor = 1;
        for(int i = 0; i < cb->dimension; i++) {
            int multiplicand_offset = (lookup_offset / index_divisor) % vq->lookup_values;
            FIX element = 0;

            if(vq->lookup_mode == 8) {
                element = setup_get_byte(vq->table + multiplicand_offset) * vq->delta_value + vq->minimum_value + last;
            } else {
                element = setup_get_short(vq->table + multiplicand_offset * 2) * vq->delta_value + vq->minimum_value + last;
            }

            if(vq->sequence_p) {
                last = element;
            }

            v[((offset + i) / period) + ((offset + i) % period) * step] += element;
            index_divisor *= vq->lookup_values;
        }
    } break;
    case 2: {
        FIX last = 0;
        int multiplicand_offset = lookup_offset * cb->dimension;
        for(int i = 0; i < cb->dimension; i++) {
            FIX element = 0;

            switch(vq->lookup_mode) {
            case 8:
                element = setup_get_byte(vq->table + multiplicand_offset) * vq->delta_value + vq->minimum_value + last;
                break;
            case 16:
                element = setup_get_short(vq->table + multiplicand_offset * 2) * vq->delta_value + vq->minimum_value + last;
                break;
            }

            if(vq->sequence_p) {
                last = element;
            }

            v[((offset + i) / period) + ((offset + i) % period) * step] += element;
            multiplicand_offset++;
        }
    } break;
    }
    return cb->dimension;
}
