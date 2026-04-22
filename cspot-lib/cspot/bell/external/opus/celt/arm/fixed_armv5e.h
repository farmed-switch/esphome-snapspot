

#ifndef FIXED_ARMv5E_H
#define FIXED_ARMv5E_H

#include "fixed_armv4.h"

#undef MULT16_32_Q16
static OPUS_INLINE opus_val32 MULT16_32_Q16_armv5e(opus_val16 a, opus_val32 b)
{
  int res;
  __asm__(
      "#MULT16_32_Q16\n\t"
      "smulwb %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(b),"r"(a)
  );
  return res;
}
#define MULT16_32_Q16(a, b) (MULT16_32_Q16_armv5e(a, b))

#undef MULT16_32_Q15
static OPUS_INLINE opus_val32 MULT16_32_Q15_armv5e(opus_val16 a, opus_val32 b)
{
  int res;
  __asm__(
      "#MULT16_32_Q15\n\t"
      "smulwb %0, %1, %2\n\t"
      : "=r"(res)
      : "r"(b), "r"(a)
  );
  return SHL32(res,1);
}
#define MULT16_32_Q15(a, b) (MULT16_32_Q15_armv5e(a, b))

#undef MAC16_32_Q15
static OPUS_INLINE opus_val32 MAC16_32_Q15_armv5e(opus_val32 c, opus_val16 a,
 opus_val32 b)
{
  int res;
  __asm__(
      "#MAC16_32_Q15\n\t"
      "smlawb %0, %1, %2, %3;\n"
      : "=r"(res)
      : "r"(SHL32(b,1)), "r"(a), "r"(c)
  );
  return res;
}
#define MAC16_32_Q15(c, a, b) (MAC16_32_Q15_armv5e(c, a, b))

#undef MAC16_32_Q16
static OPUS_INLINE opus_val32 MAC16_32_Q16_armv5e(opus_val32 c, opus_val16 a,
 opus_val32 b)
{
  int res;
  __asm__(
      "#MAC16_32_Q16\n\t"
      "smlawb %0, %1, %2, %3;\n"
      : "=r"(res)
      : "r"(b), "r"(a), "r"(c)
  );
  return res;
}
#define MAC16_32_Q16(c, a, b) (MAC16_32_Q16_armv5e(c, a, b))

#undef MAC16_16
static OPUS_INLINE opus_val32 MAC16_16_armv5e(opus_val32 c, opus_val16 a,
 opus_val16 b)
{
  int res;
  __asm__(
      "#MAC16_16\n\t"
      "smlabb %0, %1, %2, %3;\n"
      : "=r"(res)
      : "r"(a), "r"(b), "r"(c)
  );
  return res;
}
#define MAC16_16(c, a, b) (MAC16_16_armv5e(c, a, b))

#undef MULT16_16
static OPUS_INLINE opus_val32 MULT16_16_armv5e(opus_val16 a, opus_val16 b)
{
  int res;
  __asm__(
      "#MULT16_16\n\t"
      "smulbb %0, %1, %2;\n"
      : "=r"(res)
      : "r"(a), "r"(b)
  );
  return res;
}
#define MULT16_16(a, b) (MULT16_16_armv5e(a, b))

#ifdef OPUS_ARM_INLINE_MEDIA

#undef SIG2WORD16
static OPUS_INLINE opus_val16 SIG2WORD16_armv6(opus_val32 x)
{
   celt_sig res;
   __asm__(
       "#SIG2WORD16\n\t"
       "ssat %0, #16, %1, ASR #12\n\t"
       : "=r"(res)
       : "r"(x+2048)
   );
   return EXTRACT16(res);
}
#define SIG2WORD16(x) (SIG2WORD16_armv6(x))

#endif

#endif
