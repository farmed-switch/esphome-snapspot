

#ifndef CELT_LPC_SSE_H
#define CELT_LPC_SSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)
#define OVERRIDE_CELT_FIR

void celt_fir_sse4_1(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         int arch);

#if defined(OPUS_X86_PRESUME_SSE4_1)
#define celt_fir(x, num, y, N, ord, arch) \
    ((void)arch, celt_fir_sse4_1(x, num, y, N, ord, arch))

#else

extern void (*const CELT_FIR_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         int arch);

#  define celt_fir(x, num, y, N, ord, arch) \
    ((*CELT_FIR_IMPL[(arch) & OPUS_ARCHMASK])(x, num, y, N, ord, arch))

#endif
#endif

#endif
