#ifndef VORBIS_TABLE_H
#define VORBIS_TABLE_H

extern const UNSIGNED_COEFF_TYPE floor1_inverse_dB_table[256];
extern const UNSIGNED_COEFF_TYPE *vwin32_2048[7];
extern const UNSIGNED_COEFF_TYPE *cosine_table0_1024[12];
extern const UNSIGNED_COEFF_TYPE *sine_table0_1024[12];
extern const SIGNED_COEFF_TYPE FIR_coeff[160][32];

extern const uint16_t reverse_2048[2048];
extern const uint8_t FIR_offset[160];

#endif
