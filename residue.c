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

    uint8_t cascade[64];

    for(int i = 0; i < residue->classifications; i++) {
        uint8_t high_bits = 0;
        uint8_t low_bits = read_unsigned_value(3);
        uint8_t bitflag = read_unsigned_value(1);
        if(bitflag)
            high_bits = read_unsigned_value(5);
        cascade[i] = (high_bits << 3) | low_bits;
    }

    residue->class_list = setup_allocate_natural(sizeof(residue_class_t) * residue->classifications);

    residue_class_t *class_list = setup_ref(residue->class_list);

    for(int i = 0; i < residue->classifications; i++) {
        for(int j = 0; j < 8; j++) {
            if(cascade[i] & (1 << j)) {
                class_list[i].books[j] = read_unsigned_value(8);
            } else {
                class_list[i].books[j] = 0;
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

void decode_residue(int n, int index, int channel)
{
}
