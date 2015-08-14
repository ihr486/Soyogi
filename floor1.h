#ifndef FLOOR_H
#define FLOOR_H

typedef struct floor1_partition_tag {
    uint8_t class;
} floor1_partition_t;

typedef struct floor1_header_tag {
    uint8_t partitions;
    uint8_t classes;
    uint16_t partition_list;
    uint16_t class_list;
    uint8_t multiplier;
    uint8_t rangebits;
    uint16_t X_list;
    uint16_t values;
} floor1_header_t;

typedef struct floor1_subclass_tag {
    int8_t books;
} floor1_subclass_t;

typedef struct floor1_class_tag {
    uint8_t dimension;
    uint8_t subclasses;
    uint8_t masterbooks;
    uint16_t subclass_list;
} floor1_class_t;

typedef struct floor1_coord_tag {
    uint16_t X;
    uint8_t Y;
    uint8_t step2_flag;
} floor1_coord_t;

extern int floor_num;
extern floor1_header_t *floor_list;

void setup_floors(void);
void decode_floor1(int index, int channel);

#endif
