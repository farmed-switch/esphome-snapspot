

#if !defined(MDCT_ARM_H)
#define MDCT_ARM_H

#include "mdct.h"

#if defined(HAVE_ARM_NE10)

void clt_mdct_forward_neon(const mdct_lookup *l, kiss_fft_scalar *in,
                           kiss_fft_scalar * OPUS_RESTRICT out,
                           const opus_val16 *window, int overlap,
                           int shift, int stride, int arch);

void clt_mdct_backward_neon(const mdct_lookup *l, kiss_fft_scalar *in,
                            kiss_fft_scalar * OPUS_RESTRICT out,
                            const opus_val16 *window, int overlap,
                            int shift, int stride, int arch);

#if !defined(OPUS_HAVE_RTCD)
#define OVERRIDE_OPUS_MDCT (1)
#define clt_mdct_forward(_l, _in, _out, _window, _int, _shift, _stride, _arch) \
      clt_mdct_forward_neon(_l, _in, _out, _window, _int, _shift, _stride, _arch)
#define clt_mdct_backward(_l, _in, _out, _window, _int, _shift, _stride, _arch) \
      clt_mdct_backward_neon(_l, _in, _out, _window, _int, _shift, _stride, _arch)
#endif
#endif

#endif
