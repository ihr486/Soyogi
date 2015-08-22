#ifndef DECODER_H
#define DECODER_H

/*#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 2
#define __USE_POSIX2*/

#undef __STRICT_ANSI__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <setjmp.h>
#include <time.h>

#include "misc.h"
#include "vorbis_helper.h"
#include "setup.h"
#include "mode.h"
#include "mapping.h"
#include "floor1.h"
#include "residue.h"
#include "vector.h"
#include "codebook.h"
#include "vorbis_table.h"
#include "ogg.h"

//#include <pulse/pulseaudio.h>
//#include <pulse/simple.h>

void decode(void);

uint8_t read_unsigned_byte(void);
uint16_t read_unsigned_short(void);
uint32_t read_unsigned_long(void);
uint64_t read_unsigned_long_long(void);

extern bool EOF_flag;

void decode_packet(void);

void FDCT_IV(FIX *X, int V_N_bits);

extern uint8_t audio_channels;
extern uint16_t B_N_bits[2], B_N[2];

extern FILE *sox;
//extern pa_simple *pulse_ctx;

#define SECTOR_SIZE (2048)

#endif
