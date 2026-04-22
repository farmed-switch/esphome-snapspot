

#ifndef FIXED_GENERIC_H
#define FIXED_GENERIC_H

#define MULT16_16SU(a,b) ((opus_val32)(opus_val16)(a)*(opus_val32)(opus_uint16)(b))

#if OPUS_FAST_INT64
#define MULT16_32_Q16(a,b) ((opus_val32)SHR((opus_int64)((opus_val16)(a))*(b),16))
#else
#define MULT16_32_Q16(a,b) ADD32(MULT16_16((a),SHR((b),16)), SHR(MULT16_16SU((a),((b)&0x0000ffff)),16))
#endif

#if OPUS_FAST_INT64
#define MULT16_32_P16(a,b) ((opus_val32)PSHR((opus_int64)((opus_val16)(a))*(b),16))
#else
#define MULT16_32_P16(a,b) ADD32(MULT16_16((a),SHR((b),16)), PSHR(MULT16_16SU((a),((b)&0x0000ffff)),16))
#endif

#if OPUS_FAST_INT64
#define MULT16_32_Q15(a,b) ((opus_val32)SHR((opus_int64)((opus_val16)(a))*(b),15))
#else
#define MULT16_32_Q15(a,b) ADD32(SHL(MULT16_16((a),SHR((b),16)),1), SHR(MULT16_16SU((a),((b)&0x0000ffff)),15))
#endif

#if OPUS_FAST_INT64
#define MULT32_32_Q16(a,b) ((opus_val32)SHR((opus_int64)(a)*(opus_int64)(b),16))
#else
#define MULT32_32_Q16(a,b) (ADD32(ADD32(ADD32((opus_val32)(SHR32(((opus_uint32)((a)&0x0000ffff)*(opus_uint32)((b)&0x0000ffff)),16)), MULT16_16SU(SHR32(a,16),((b)&0x0000ffff))), MULT16_16SU(SHR32(b,16),((a)&0x0000ffff))), SHL32(MULT16_16(SHR32(a,16),SHR32(b,16)),16)))
#endif

#if OPUS_FAST_INT64
#define MULT32_32_Q31(a,b) ((opus_val32)SHR((opus_int64)(a)*(opus_int64)(b),31))
#else
#define MULT32_32_Q31(a,b) ADD32(ADD32(SHL(MULT16_16(SHR((a),16),SHR((b),16)),1), SHR(MULT16_16SU(SHR((a),16),((b)&0x0000ffff)),15)), SHR(MULT16_16SU(SHR((b),16),((a)&0x0000ffff)),15))
#endif

#define QCONST16(x,bits) ((opus_val16)(.5+(x)*(((opus_val32)1)<<(bits))))

#define QCONST32(x,bits) ((opus_val32)(.5+(x)*(((opus_val32)1)<<(bits))))

#define NEG16(x) (-(x))

#define NEG32(x) (-(x))

#define EXTRACT16(x) ((opus_val16)(x))

#define EXTEND32(x) ((opus_val32)(x))

#define SHR16(a,shift) ((a) >> (shift))

#define SHL16(a,shift) ((opus_int16)((opus_uint16)(a)<<(shift)))

#define SHR32(a,shift) ((a) >> (shift))

#define SHL32(a,shift) ((opus_int32)((opus_uint32)(a)<<(shift)))

#define PSHR32(a,shift) (SHR32((a)+((EXTEND32(1)<<((shift))>>1)),shift))

#define VSHR32(a, shift) (((shift)>0) ? SHR32(a, shift) : SHL32(a, -(shift)))

#define SHR(a,shift) ((a) >> (shift))
#define SHL(a,shift) SHL32(a,shift)
#define PSHR(a,shift) (SHR((a)+((EXTEND32(1)<<((shift))>>1)),shift))
#define SATURATE(x,a) (((x)>(a) ? (a) : (x)<-(a) ? -(a) : (x)))

#define SATURATE16(x) (EXTRACT16((x)>32767 ? 32767 : (x)<-32768 ? -32768 : (x)))

#define ROUND16(x,a) (EXTRACT16(PSHR32((x),(a))))

#define SROUND16(x,a) EXTRACT16(SATURATE(PSHR32(x,a), 32767));

#define HALF16(x)  (SHR16(x,1))
#define HALF32(x)  (SHR32(x,1))

#define ADD16(a,b) ((opus_val16)((opus_val16)(a)+(opus_val16)(b)))

#define SUB16(a,b) ((opus_val16)(a)-(opus_val16)(b))

#define ADD32(a,b) ((opus_val32)(a)+(opus_val32)(b))

#define SUB32(a,b) ((opus_val32)(a)-(opus_val32)(b))

#define ADD32_ovflw(a,b) ((opus_val32)((opus_uint32)(a)+(opus_uint32)(b)))

#define SUB32_ovflw(a,b) ((opus_val32)((opus_uint32)(a)-(opus_uint32)(b)))

#define NEG32_ovflw(a) ((opus_val32)(0-(opus_uint32)(a)))

#define MULT16_16_16(a,b)     ((((opus_val16)(a))*((opus_val16)(b))))

#define MULT32_32_32(a,b)     ((((opus_val32)(a))*((opus_val32)(b))))

#define MULT16_16(a,b)     (((opus_val32)(opus_val16)(a))*((opus_val32)(opus_val16)(b)))

#define MAC16_16(c,a,b) (ADD32((c),MULT16_16((a),(b))))

#define MAC16_32_Q15(c,a,b) ADD32((c),ADD32(MULT16_16((a),SHR((b),15)), SHR(MULT16_16((a),((b)&0x00007fff)),15)))

#define MAC16_32_Q16(c,a,b) ADD32((c),ADD32(MULT16_16((a),SHR((b),16)), SHR(MULT16_16SU((a),((b)&0x0000ffff)),16)))

#define MULT16_16_Q11_32(a,b) (SHR(MULT16_16((a),(b)),11))
#define MULT16_16_Q11(a,b) (SHR(MULT16_16((a),(b)),11))
#define MULT16_16_Q13(a,b) (SHR(MULT16_16((a),(b)),13))
#define MULT16_16_Q14(a,b) (SHR(MULT16_16((a),(b)),14))
#define MULT16_16_Q15(a,b) (SHR(MULT16_16((a),(b)),15))

#define MULT16_16_P13(a,b) (SHR(ADD32(4096,MULT16_16((a),(b))),13))
#define MULT16_16_P14(a,b) (SHR(ADD32(8192,MULT16_16((a),(b))),14))
#define MULT16_16_P15(a,b) (SHR(ADD32(16384,MULT16_16((a),(b))),15))

#define DIV32_16(a,b) ((opus_val16)(((opus_val32)(a))/((opus_val16)(b))))

#define DIV32(a,b) (((opus_val32)(a))/((opus_val32)(b)))

#if defined(MIPSr1_ASM)
#include "mips/fixed_generic_mipsr1.h"
#endif

static OPUS_INLINE opus_val16 SIG2WORD16_generic(celt_sig x)
{
   x = PSHR32(x, SIG_SHIFT);
   x = MAX32(x, -32768);
   x = MIN32(x, 32767);
   return EXTRACT16(x);
}
#define SIG2WORD16(x) (SIG2WORD16_generic(x))

#endif
