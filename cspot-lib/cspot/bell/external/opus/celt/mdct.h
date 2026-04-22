

#ifndef MDCT_H
#define MDCT_H

#include "opus_defines.h"
#include "kiss_fft.h"
#include "arch.h"

typedef struct {
   int n;
   int maxshift;
   const kiss_fft_state *kfft[4];
   const kiss_twiddle_scalar * OPUS_RESTRICT trig;
} mdct_lookup;

#if defined(HAVE_ARM_NE10)
#include "arm/mdct_arm.h"
#endif

int clt_mdct_init(mdct_lookup *l,int N, int maxshift, int arch);
void clt_mdct_clear(mdct_lookup *l, int arch);

void clt_mdct_forward_c(const mdct_lookup *l, kiss_fft_scalar *in,
                        kiss_fft_scalar * OPUS_RESTRICT out,
                        const opus_val16 *window, int overlap,
                        int shift, int stride, int arch);

void clt_mdct_backward_c(const mdct_lookup *l, kiss_fft_scalar *in,
      kiss_fft_scalar * OPUS_RESTRICT out,
      const opus_val16 * OPUS_RESTRICT window,
      int overlap, int shift, int stride, int arch);

#if !defined(OVERRIDE_OPUS_MDCT)

#if defined(OPUS_HAVE_RTCD) && defined(HAVE_ARM_NE10)

extern void (*const CLT_MDCT_FORWARD_IMPL[OPUS_ARCHMASK+1])(
      const mdct_lookup *l, kiss_fft_scalar *in,
      kiss_fft_scalar * OPUS_RESTRICT out, const opus_val16 *window,
      int overlap, int shift, int stride, int arch);

#define clt_mdct_forward(_l, _in, _out, _window, _overlap, _shift, _stride, _arch) \
   ((*CLT_MDCT_FORWARD_IMPL[(arch)&OPUS_ARCHMASK])(_l, _in, _out, \
                                                   _window, _overlap, _shift, \
                                                   _stride, _arch))

extern void (*const CLT_MDCT_BACKWARD_IMPL[OPUS_ARCHMASK+1])(
      const mdct_lookup *l, kiss_fft_scalar *in,
      kiss_fft_scalar * OPUS_RESTRICT out, const opus_val16 *window,
      int overlap, int shift, int stride, int arch);

#define clt_mdct_backward(_l, _in, _out, _window, _overlap, _shift, _stride, _arch) \
   (*CLT_MDCT_BACKWARD_IMPL[(arch)&OPUS_ARCHMASK])(_l, _in, _out, \
                                                   _window, _overlap, _shift, \
                                                   _stride, _arch)

#else

#define clt_mdct_forward(_l, _in, _out, _window, _overlap, _shift, _stride, _arch) \
   clt_mdct_forward_c(_l, _in, _out, _window, _overlap, _shift, _stride, _arch)

#define clt_mdct_backward(_l, _in, _out, _window, _overlap, _shift, _stride, _arch) \
   clt_mdct_backward_c(_l, _in, _out, _window, _overlap, _shift, _stride, _arch)

#endif
#endif

#endif
