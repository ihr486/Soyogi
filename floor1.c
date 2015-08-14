#include "decoder.h"

int floor_num = 0;
floor1_header_t *floor_list = NULL;
floor1_vector_t *floor_vector_list = NULL;

static void setup_floor1(int index);

void setup_floors(void)
{
    floor_num = read_unsigned_value(6) + 1;
    floor_list = setup_ref(setup_allocate_natural(sizeof(floor1_header_t) * floor_num));

    for(int i = 0; i < floor_num; i++) {
        uint16_t floor_type = read_unsigned_value(16);

        switch(floor_type) {
        case 0:
            ERROR(ERROR_FLOOR, "This decoder does not support Floor 0.\n");
        case 1:
            setup_floor1(i);
            break;
        default:
            ERROR(ERROR_FLOOR, "Unknown floor type: %d.\n", floor_type);
        }
    }

    INFO("%d floor configurations decoded.\n", floor_num);

    floor_vector_list = setup_ref(setup_allocate_natural(sizeof(floor1_vector_t) * audio_channels));
}

static void setup_floor1(int index)
{
    floor1_header_t *f = &floor_list[index];

    f->partitions = read_unsigned_value(5);
    f->partition_list = setup_allocate_natural(sizeof(floor1_partition_t) * f->partitions);

    floor1_partition_t *partition_list = setup_ref(f->partition_list);
    int maximum_class = -1;

    for(int i = 0; i < f->partitions; i++) {
        partition_list[i].class = read_unsigned_value(4);

        if(partition_list[i].class > maximum_class) maximum_class = partition_list[i].class;
    }

    if(maximum_class < 0)
        ERROR(ERROR_FLOOR1, "No class information for Floor #%d?\n", index);

    f->classes = maximum_class + 1;
    f->class_list = setup_allocate_natural(sizeof(floor1_class_t) * f->classes);

    floor1_class_t *class_list = setup_ref(f->class_list);

    for(int i = 0; i < f->classes; i++) {
        class_list[i].dimension = read_unsigned_value(3) + 1;
        class_list[i].subclasses = read_unsigned_value(2);
        class_list[i].masterbooks = 0;
        if(class_list[i].subclasses) {
            class_list[i].masterbooks = read_unsigned_value(8);
        }
        class_list[i].subclass_list = setup_allocate_natural(sizeof(floor1_subclass_t) * (1 << class_list[i].subclasses));

        floor1_subclass_t *subclass_list = setup_ref(class_list[i].subclass_list);

        for(int j = 0; j < (1 << class_list[i].subclasses); j++) {
            subclass_list[j].books = read_unsigned_value(8) - 1;
        }
    }

    f->multiplier = read_unsigned_value(2) + 1;
    f->rangebits = read_unsigned_value(4);

    f->X_list = setup_allocate_natural(sizeof(uint16_t) * 2);

    setup_set_short(f->X_list, 0);
    setup_set_short(f->X_list + 2, 1 << f->rangebits);

    f->values = 2;
    for(int i = 0; i < f->partitions; i++) {
        int current_class_number = partition_list[i].class;

        for(int j = 0; j < class_list[current_class_number].dimension; j++) {
            uint16_t X_value = read_unsigned_value(f->rangebits);
            printf("%u ", X_value);
            setup_push_short(X_value);
            f->values++;
        }
    }
    printf("\n");
}

void decode_floor1(int index, int channel)
{
    static const int range_list[4] = {256, 128, 86, 64};

    floor1_header_t *floor = &floor_list[index];

    int nonzero = read_unsigned_value(1);

    if(nonzero) {
        int range = range_list[floor->multiplier - 1];

        floor1_vector_t *vector = &floor_vector_list[channel];

        vector->Y_list = setup_allocate_natural(sizeof(uint8_t) * floor->values);
        vector->step2_flag_list = setup_allocate_natural(sizeof(uint8_t) * floor->values);

        setup_set_byte(vector->Y_list, read_unsigned_value(ilog(range - 1)));
        setup_set_byte(vector->Y_list + 1, read_unsigned_value(ilog(range - 1)));

        uint16_t src_Y_list = setup_allocate_natural(sizeof(uint8_t) * 2);

        setup_set_byte(src_Y_list, setup_get_byte(vector->Y_list));
        setup_set_byte(src_Y_list + 1, setup_get_byte(vector->Y_list + 1));

        uint8_t *final_Y = setup_ref(vector->Y_list);
        uint8_t *Y = setup_ref(src_Y_list);

        floor1_partition_t *partition_list = setup_ref(floor->partition_list);
        floor1_class_t *class_list = setup_ref(floor->class_list);

        for(unsigned int i = 0; i < floor->partitions; i++) {
            int class = partition_list[i].class;
            int cdim = class_list[class].dimension;
            int cbits = class_list[class].subclasses;
            int csub = (1 << cbits) - 1;
            int cval = 0;

            if(cbits > 0) {
                cval = lookup_scalar(class_list[class].masterbooks);
            }

            floor1_subclass_t *subclass_list = setup_ref(class_list[class].subclass_list);

            for(int j = 0; j < cdim; j++) {
                int book = subclass_list[cval & csub].books;
                cval >>= cbits;

                if(book >= 0) {
                    setup_push_byte(lookup_scalar(book));
                } else {
                    setup_push_byte(0);
                }
            }
        }

        setup_set_byte(vector->step2_flag_list, 1);
        setup_set_byte(vector->step2_flag_list + 1, 1);

        uint8_t *step2_flag = setup_ref(vector->step2_flag_list);
        uint16_t *X = setup_ref(floor->X_list);

        for(int i = 2; i < floor->values; i++) {
            int low_neighbor_offset = low_neighbor(X, i);
            int high_neighbor_offset = high_neighbor(X, i);

            int predicted = render_point(X[low_neighbor_offset], Y[low_neighbor_offset], X[high_neighbor_offset], Y[high_neighbor_offset], X[i]);

            int val = Y[i];
            int highroom = range - predicted;
            int lowroom = predicted;
            int room;
            if(highroom < lowroom) {
                room = highroom * 2;
            } else {
                room = lowroom * 2;
            }
            if(val) {
                step2_flag[low_neighbor_offset] = 1;
                step2_flag[high_neighbor_offset] = 1;
                step2_flag[i] = 1;
                if(val >= room) {
                    if(highroom > lowroom) {
                        final_Y[i] = val - lowroom + predicted;
                    } else {
                        final_Y[i] = predicted - val + highroom - 1;
                    }
                } else {
                    if(val & 1) {
                        final_Y[i] = predicted - (val + 1) / 2;
                    } else {
                        final_Y[i] = predicted + val / 2;
                    }
                }
            } else {
                step2_flag[i] = 0;
                final_Y[i] = predicted;
            }
        }

        printf("\t");
        for(unsigned int i = 0; i < floor->values; i++) {
            printf("(%d, %d) ", setup_get_short(floor->X_list + 2 * i), setup_get_byte(vector->Y_list + i));
        }
        printf("\n");

        setup_set_head(src_Y_list);
    }
}
