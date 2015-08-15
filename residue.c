#include "decoder.h"

int residue_num = 0;
residue_header_t *residue_list = NULL;

static void setup_residue(residue_header_t *residue)
{
    residue->begin = read_unsigned_value(24);
    residue->end = read_unsigned_value(24);
    residue->partition_size = read_unsigned_value(24) + 1;
    residue->classifications = read_unsigned_value(6) + 1;
    residue->classbook = read_unsigned_value(8);

    INFO("Residue: [%d:%d], CB = %d, %d classes.\n", residue->begin, residue->end, residue->classbook, residue->classifications);

    uint8_t cascade[64];

    for(int i = 0; i < residue->classifications; i++) {
        uint8_t high_bits = 0;
        uint8_t low_bits = read_unsigned_value(3);
        uint8_t bitflag = read_unsigned_value(1);
        if(bitflag)
            high_bits = read_unsigned_value(5);
        cascade[i] = (high_bits << 3) | low_bits;

        printf("%d ", cascade[i]);
    }
    printf("\n");

    residue->class_list = setup_allocate_natural(sizeof(residue_class_t) * residue->classifications);

    residue_class_t *class_list = setup_ref(residue->class_list);

    for(int i = 0; i < residue->classifications; i++) {
        for(int j = 0; j < 8; j++) {
            if(cascade[i] & (1 << j)) {
                class_list[i].books[j] = read_unsigned_value(8);
            } else {
                class_list[i].books[j] = 255;
            }
            printf("%d ", class_list[i].books[j]);
        }
    }
    printf("\n");
}

void setup_residues(void)
{
    residue_num = read_unsigned_value(6) + 1;
    residue_list = setup_ref(setup_allocate_natural(sizeof(residue_header_t) * residue_num));

    for(int i = 0; i < residue_num; i++) {
        residue_header_t *residue = &residue_list[i];

        residue->type = read_unsigned_value(16);

        switch(residue->type) {
        case 0:
        case 1:
        case 2:
            setup_residue(residue);
            break;
        default:
            ERROR(ERROR_RESIDUE, "Unknown residue type: %d.\n", residue->type);
        }
    }

    INFO("%d residue configurations decoded.\n", residue_num);
}

void decode_residue(int n, int index, int offset, int channel)
{
    residue_header_t *residue = &residue_list[index];

    int actual_size = n / 2;

    INFO("\tResidue type %d the size of %d*%d.\n", residue->type, actual_size, channel);

    if(residue->type == 2) {
        vector_list[offset].body = setup_allocate_natural(sizeof(float) * actual_size * channel);

        for(int i = 1; i < channel; i++) {
            vector_list[offset + i].body = vector_list[offset].body + sizeof(float) * actual_size * i;

            float *v = setup_ref(vector_list[offset + i].body);

            for(int j = 0; j < actual_size; j++) {
                v[j] = 0.0f;
            }
        }
        
        actual_size *= channel;
        channel = 1;
    } else {
        for(int i = 0; i < channel; i++) {
            vector_list[offset + i].body = setup_allocate_natural(sizeof(float) * actual_size);

            float *v = setup_ref(vector_list[offset + i].body);

            for(int j = 0; j < actual_size; j++) {
                v[j] = 0.0f;
            }
        }
    }

    int limit_residue_begin = min(residue->begin, actual_size);
    int limit_residue_end = min(residue->end, actual_size);

    int classwords_per_codeword = codebook_list[residue->classbook].dimension;
    int n_to_read = limit_residue_end - limit_residue_begin;
    int partitions_to_read = n_to_read / residue->partition_size;

    uint8_t *classifications = setup_ref(setup_allocate_packed(channel * partitions_to_read));

    INFO("\tDecoding from %d to %d.\n", limit_residue_begin, limit_residue_end);
    INFO("\tCPC = %d, %d*%d partitions.\n", classwords_per_codeword, residue->partition_size, partitions_to_read);

    if(n_to_read) {
        for(int pass = 0; pass < 8; pass++) {
            int partition_count = 0;

            while(partition_count < partitions_to_read) {
                if(pass == 0) {
                    for(int j = 0; j < channel; j++) {
                        if(!vector_list[offset + j].do_not_decode_flag) {
                            int temp = lookup_scalar(residue->classbook);

                            if(temp < 0) return;

                            for(int i = classwords_per_codeword - 1; i >= 0; i--) {
                                classifications[j * partitions_to_read + i + partition_count] = temp % residue->classifications;

                                temp /= residue->classifications;
                            }
                        }
                    }
                }
                for(int i = 0; i < classwords_per_codeword && partition_count < partitions_to_read; i++) {
                    for(int j = 0; j < channel; j++) {
                        float *v = setup_ref(vector_list[offset + j].body);
                        if(!vector_list[offset + j].do_not_decode_flag) {
                            int vqclass = classifications[j * partitions_to_read + partition_count];

                            residue_class_t *class_list = setup_ref(residue->class_list);
                            int vqbook = class_list[vqclass].books[pass];

                            if(vqbook != 255) {
                                switch(residue->type) {
                                case 0: {
                                    int step = residue->partition_size / codebook_list[vqbook].dimension;

                                    for(int k = 0; k < step; k++) {
                                        if(lookup_vector(v + (limit_residue_begin + partition_count * residue->partition_size) + k, vqbook, step) == 0) return;
                                    }
                                } break;
                                case 1:
                                case 2: {
                                    for(int k = 0; k < residue->partition_size; k += codebook_list[vqbook].dimension) {
                                        if(lookup_vector(v + (limit_residue_begin + partition_count * residue->partition_size) + k, vqbook, 1) == 0) return;
                                    }
                                } break;
                                }
                            }
                        } else {
                            INFO("\tSkipping vector %d.\n", j);
                        }
                    }
                    partition_count++;
                }
            }
        }
    }

    if(residue->type == 2) {

    }
}
