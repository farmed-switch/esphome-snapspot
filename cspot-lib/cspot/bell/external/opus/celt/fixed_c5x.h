

#ifndef FIXED_C5X_H
#define FIXED_C5X_H

#include "dsplib.h"

#undef IMUL32
static OPUS_INLINE long IMUL32(long i, long j)
{
   long ac0, ac1;
   ac0 = _lmpy(i>>16,j);
   ac1 = ac0 + _lmpy(i,j>>16);
   return _lmpyu(i,j) + (ac1<<16);
}

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

#undef MULT16_16SU
#define MULT16_16SU(a,b) _lmpysu(a,b)

#undef MULT_16_16
#define MULT_16_16(a,b) _lmpy(a,b)

#undef MULT16_32_Q15
#define MULT16_32_Q15(a,b) ADD32(SHL(MULT16_16((a),SHR((b),16)),1), SHR(MULT16_16SU((a),(b)),15))

#define celt_ilog2(x) (30 - _lnorm(x))
#define OVERRIDE_CELT_ILOG2

#define celt_maxabs16(x, len) MAX32(EXTEND32(maxval((DATA *)x, len)),-EXTEND32(minval((DATA *)x, len)))
#define OVERRIDE_CELT_MAXABS16

#endif
