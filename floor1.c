#include "decoder.h"

typedef struct floor1_header_tag {
} floor1_header_t;

typedef struct floor1_class_tag {
} floor1_class_t;

floor1_header_t *floor_list = NULL;

static void setup_floor1(void);

void setup_floors(void)
{
    int floor_count = read_unsigned_value(6) + 1;

    for(int i = 0; i < floor_count; i++) {
        uint16_t floor_type = read_unsigned_value(16);

        switch(floor_type) {
        case 0:
            ERROR(ERROR_FLOOR, "This decoder does not support Floor 0.\n");
        case 1:
            setup_floor1();
            break;
        default:
            ERROR(ERROR_FLOOR, "Unknown floor type: %d.\n", floor_type);
        }
    }

    INFO("%d floor configurations decoded.\n", floor_count);
}

static void setup_floor1(void)
{
    int partitions = read_unsigned_value(5);
    int maximum_class = -1;

    for(int i = 0; i < partitions; i++) {
        int class = read_unsigned_value(4);

        if(class > maximum_class) maximum_class = class;
    }

    for(int i = 0; i <= maximum_class; i++) {
        int dimension = read_unsigned_value(3) + 1;
        int subclasses = read_unsigned_value(2);
        int masterbooks = 0;
        if(subclasses) {
            masterbooks = read_unsigned_value(8);
        }
        for(int j = 0; j < (1 << subclasses); j++) {
            int books = read_unsigned_value(8) - 1;
        }
    }

    int multiplier = read_unsigned_value(2) + 1;
    int rangebits = read_unsigned_value(4);
}

/*void decode_floor1(void)
{
}*/
