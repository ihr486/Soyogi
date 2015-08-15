#ifndef MODE_H
#define MODE_H

typedef struct vorbis_mode_tag {
    uint8_t blockflag;
    uint8_t mapping;
} vorbis_mode_t;

extern int mode_num;
extern vorbis_mode_t *mode_list;

void setup_modes(void);

#endif
