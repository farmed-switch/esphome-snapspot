

#ifndef CPU_SUPPORT_H
#define CPU_SUPPORT_H

#include "opus_types.h"
#include "opus_defines.h"

#if defined(OPUS_HAVE_RTCD) && \
  (defined(OPUS_ARM_ASM) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR))
#include "arm/armcpu.h"

#define OPUS_ARCHMASK 3

#elif (defined(OPUS_X86_MAY_HAVE_SSE) && !defined(OPUS_X86_PRESUME_SSE)) || \
  (defined(OPUS_X86_MAY_HAVE_SSE2) && !defined(OPUS_X86_PRESUME_SSE2)) || \
  (defined(OPUS_X86_MAY_HAVE_SSE4_1) && !defined(OPUS_X86_PRESUME_SSE4_1)) || \
  (defined(OPUS_X86_MAY_HAVE_AVX) && !defined(OPUS_X86_PRESUME_AVX))

#include "x86/x86cpu.h"

#define OPUS_ARCHMASK 7
int opus_select_arch(void);

#else
#define OPUS_ARCHMASK 0

static OPUS_INLINE int opus_select_arch(void)
{
  return 0;
}
#endif
#endif
