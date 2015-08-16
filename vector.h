#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector_tag {
    uint8_t floor;
    uint8_t nonzero : 1;
    uint8_t no_residue : 1;
    uint8_t do_not_decode_flag : 1;
    uint8_t next_window_flag : 1;
    uint16_t coord_list;
    uint16_t right_hand;
    uint16_t body;
} vector_t;

void setup_vectors(void);
void decouple_square_polar(int V_N, int magnitude, int angle);
void cache_righthand(int V_N, int channel, int next_window_flag);
void overlap_add(int V_N_bits, int channel, int previous_window_flag);

extern vector_t *vector_list;

#endif
