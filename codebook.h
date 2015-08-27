#ifndef CODEBOOK_H
#define CODEBOOK_H

typedef struct codebook_tag {
    uint16_t entries;
    uint8_t dimension;
    uint8_t lookup_type;
    uint16_t huffman;
    uint16_t lookup;
} codebook_t;

typedef struct VQ_header_tag {
    DATA_TYPE minimum_value;
    DATA_TYPE delta_value;
    uint8_t sequence_p;
    uint8_t lookup_mode;
    uint16_t lookup_values;
    uint16_t table;
} VQ_header_t;

extern int codebook_num;
extern codebook_t *codebook_list;

void setup_codebooks(void);
int16_t lookup_scalar(int index);
int lookup_vector(DATA_TYPE *v, int offset, int index, int step, int period);

#endif
