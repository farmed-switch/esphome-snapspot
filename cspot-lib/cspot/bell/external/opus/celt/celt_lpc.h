

#ifndef PLC_H
#define PLC_H

#include "arch.h"
#include "cpu_support.h"

#if defined(OPUS_X86_MAY_HAVE_SSE4_1)
#include "x86/celt_lpc_sse.h"
#endif

#define LPC_ORDER 24

void _celt_lpc(opus_val16 *_lpc, const opus_val32 *ac, int p);

void celt_fir_c(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16 *y,
         int N,
         int ord,
         int arch);

#if !defined(OVERRIDE_CELT_FIR)
#define celt_fir(x, num, y, N, ord, arch) \
    (celt_fir_c(x, num, y, N, ord, arch))
#endif

void celt_iir(const opus_val32 *x,
         const opus_val16 *den,
         opus_val32 *y,
         int N,
         int ord,
         opus_val16 *mem,
         int arch);

int _celt_autocorr(const opus_val16 *x, opus_val32 *ac,
         const opus_val16 *window, int overlap, int lag, int n, int arch);

#endif
