

#ifndef FIXED_C6X_H
#define FIXED_C6X_H

#undef MULT16_16SU
#define MULT16_16SU(a,b) _mpysu(a,b)

#undef MULT_16_16
#define MULT_16_16(a,b) _mpy(a,b)

#define celt_ilog2(x) (30 - _norm(x))
#define OVERRIDE_CELT_ILOG2

#undef MULT16_32_Q15
#define MULT16_32_Q15(a,b) (_mpylill(a, b) >> 15)

#if 0
#include "dsplib.h"

#undef MAX16
#define MAX16(a,b) _max(a,b)

#undef MIN16
#define MIN16(a,b) _min(a,b)

#undef MAX32
#define MAX32(a,b) _lmax(a,b)

#undef MIN32
#define MIN32(a,b) _lmin(a,b)

#undef VSHR32
#define VSHR32(a, shift) _lshl(a,-(shift))

#undef MULT16_16_Q15
#define MULT16_16_Q15(a,b) (_smpy(a,b))

#define celt_maxabs16(x, len) MAX32(EXTEND32(maxval((DATA *)x, len)),-EXTEND32(minval((DATA *)x, len)))
#define OVERRIDE_CELT_MAXABS16

#endif
