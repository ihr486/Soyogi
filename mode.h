#ifndef MODE_H
#define MODE_H

typedef struct mode_tag {
    uint8_t blockflag;
    uint8_t mapping;
} mode_t;

extern int mode_num;
extern mode_t *mode_list;

void setup_modes(void);

#endif
