#ifndef DECODER_H
#define DECODER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>

#include "misc.h"
#include "vorbis_helper.h"
#include "setup.h"
#include "mode.h"
#include "mapping.h"
#include "floor1.h"
#include "residue.h"
#include "vector.h"
#include "codebook.h"

#define SECTOR_SIZE (512)

void decode(void);

uint8_t read_unsigned_byte(void);
uint16_t read_unsigned_short(void);
uint32_t read_unsigned_long(void);
uint64_t read_unsigned_long_long(void);

bool reached_EOF(void);
extern volatile bool EOP_flag;

uint32_t read_unsigned_value(int n);
int32_t read_signed_value(int n);
float read_float32(void);

int decode_packet(void);

void FDCT_IV(float *X, int n);

extern const float floor1_inverse_dB_table[256];

extern uint8_t audio_channels;
extern uint16_t blocksize[2];
extern uint32_t audio_sample_rate;
extern uint32_t bitrate_nominal;

#endif
