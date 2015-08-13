#ifndef MAPPING_H
#define MAPPING_H

typedef struct coupling_step_tag {
    uint8_t magnitude;
    uint8_t angle;
} coupling_step_t;

typedef struct submap_tag {
    uint8_t floor;
    uint8_t residue;
} submap_t;

typedef struct channel_tag {
    uint8_t mux;
} channel_t;

typedef struct mapping_header_tag {
    uint8_t submaps;
    uint8_t channels;
    uint16_t submap_list;
    uint16_t channel_list;
    uint16_t coupling_steps;
    uint16_t coupling_step_list;
} mapping_header_t;

extern int mapping_num;
extern mapping_header_t *mapping_list;

void setup_mappings(void);

#endif
