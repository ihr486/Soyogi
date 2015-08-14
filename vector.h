#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector_tag {
    uint8_t floor;
    uint8_t nonzero : 1;
    uint8_t do_not_decode_flag : 1;
    uint16_t coord_list;
} vector_t;

void setup_vectors(void);
void decouple_square_polar(int n, int magnitude, int angle);

extern vector_t *vector_list;

#endif
