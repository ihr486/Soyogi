#ifndef RESIDUE_H
#define RESIDUE_H

typedef struct residue_class_tag {
    uint8_t books[8];
} residue_class_t;

typedef struct residue_header_tag {
    uint8_t type;
    uint16_t begin, end;
    uint16_t partition_size;
    uint8_t classifications;
    uint8_t classbook;
    uint16_t class_list;
} residue_header_t;

typedef struct residue_vector_tag {
    uint8_t do_not_decode_flag;
} residue_vector_t;

extern int residue_num;
extern residue_header_t *residue_list;
extern residue_vector_t *residue_vector_list;

#endif
