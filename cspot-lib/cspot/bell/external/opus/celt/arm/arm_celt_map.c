

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pitch.h"
#include "kiss_fft.h"
#include "mdct.h"

#if defined(OPUS_HAVE_RTCD)

# if defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR)
opus_val32 (*const CELT_INNER_PROD_IMPL[OPUS_ARCHMASK+1])(const opus_val16 *x, const opus_val16 *y, int N) = {
  celt_inner_prod_c,
  celt_inner_prod_c,
  celt_inner_prod_c,
  celt_inner_prod_neon
};

void (*const DUAL_INNER_PROD_IMPL[OPUS_ARCHMASK+1])(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
      int N, opus_val32 *xy1, opus_val32 *xy2) = {
  dual_inner_prod_c,
  dual_inner_prod_c,
  dual_inner_prod_c,
  dual_inner_prod_neon
};
# endif

# if defined(FIXED_POINT)
#  if ((defined(OPUS_ARM_MAY_HAVE_NEON) && !defined(OPUS_ARM_PRESUME_NEON)) || \
    (defined(OPUS_ARM_MAY_HAVE_MEDIA) && !defined(OPUS_ARM_PRESUME_MEDIA)) || \
    (defined(OPUS_ARM_MAY_HAVE_EDSP) && !defined(OPUS_ARM_PRESUME_EDSP)))
opus_val32 (*const CELT_PITCH_XCORR_IMPL[OPUS_ARCHMASK+1])(const opus_val16 *,
    const opus_val16 *, opus_val32 *, int, int, int) = {
  celt_pitch_xcorr_c,
  MAY_HAVE_EDSP(celt_pitch_xcorr),
  MAY_HAVE_MEDIA(celt_pitch_xcorr),
  MAY_HAVE_NEON(celt_pitch_xcorr)
};

#  endif
# else
#  if defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR)
void (*const CELT_PITCH_XCORR_IMPL[OPUS_ARCHMASK+1])(const opus_val16 *,
    const opus_val16 *, opus_val32 *, int, int, int) = {
  celt_pitch_xcorr_c,
  celt_pitch_xcorr_c,
  celt_pitch_xcorr_c,
  celt_pitch_xcorr_float_neon
};
#  endif
# endif

#if defined(FIXED_POINT) && defined(OPUS_HAVE_RTCD) && \
 defined(OPUS_ARM_MAY_HAVE_NEON_INTR) && !defined(OPUS_ARM_PRESUME_NEON_INTR)

void (*const XCORR_KERNEL_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *y,
         opus_val32       sum[4],
         int              len
) = {
  xcorr_kernel_c,
  xcorr_kernel_c,
  xcorr_kernel_c,
  xcorr_kernel_neon_fixed,
};

#endif

# if defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
#  if defined(HAVE_ARM_NE10)
#   if defined(CUSTOM_MODES)
int (*const OPUS_FFT_ALLOC_ARCH_IMPL[OPUS_ARCHMASK+1])(kiss_fft_state *st) = {
   opus_fft_alloc_arch_c,
   opus_fft_alloc_arch_c,
   opus_fft_alloc_arch_c,
   opus_fft_alloc_arm_neon
};

void (*const OPUS_FFT_FREE_ARCH_IMPL[OPUS_ARCHMASK+1])(kiss_fft_state *st) = {
   opus_fft_free_arch_c,
   opus_fft_free_arch_c,
   opus_fft_free_arch_c,
   opus_fft_free_arm_neon
};
#   endif

void (*const OPUS_FFT[OPUS_ARCHMASK+1])(const kiss_fft_state *cfg,
                                        const kiss_fft_cpx *fin,
                                        kiss_fft_cpx *fout) = {
   opus_fft_c,
   opus_fft_c,
   opus_fft_c,
   opus_fft_neon
};

void (*const OPUS_IFFT[OPUS_ARCHMASK+1])(const kiss_fft_state *cfg,
                                         const kiss_fft_cpx *fin,
                                         kiss_fft_cpx *fout) = {
   opus_ifft_c,
   opus_ifft_c,
   opus_ifft_c,
   opus_ifft_neon
};

void (*const CLT_MDCT_FORWARD_IMPL[OPUS_ARCHMASK+1])(const mdct_lookup *l,
                                                     kiss_fft_scalar *in,
                                                     kiss_fft_scalar * OPUS_RESTRICT out,
                                                     const opus_val16 *window,
                                                     int overlap, int shift,
                                                     int stride, int arch) = {
   clt_mdct_forward_c,
   clt_mdct_forward_c,
   clt_mdct_forward_c,
   clt_mdct_forward_neon
};

void (*const CLT_MDCT_BACKWARD_IMPL[OPUS_ARCHMASK+1])(const mdct_lookup *l,
                                                      kiss_fft_scalar *in,
                                                      kiss_fft_scalar * OPUS_RESTRICT out,
                                                      const opus_val16 *window,
                                                      int overlap, int shift,
                                                      int stride, int arch) = {
   clt_mdct_backward_c,
   clt_mdct_backward_c,
   clt_mdct_backward_c,
   clt_mdct_backward_neon
};

#  endif
# endif

#endif
