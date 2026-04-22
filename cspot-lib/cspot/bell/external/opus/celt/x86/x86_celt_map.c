

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "x86/x86cpu.h"
#include "celt_lpc.h"
#include "pitch.h"
#include "pitch_sse.h"
#include "vq.h"

#if defined(OPUS_HAVE_RTCD)

# if defined(FIXED_POINT)

#if defined(OPUS_X86_MAY_HAVE_SSE4_1) && !defined(OPUS_X86_PRESUME_SSE4_1)

void (*const CELT_FIR_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *num,
         opus_val16       *y,
         int              N,
         int              ord,
         int              arch
) = {
  celt_fir_c,
  celt_fir_c,
  celt_fir_c,
  MAY_HAVE_SSE4_1(celt_fir),
  MAY_HAVE_SSE4_1(celt_fir)
};

void (*const XCORR_KERNEL_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *y,
         opus_val32       sum[4],
         int              len
) = {
  xcorr_kernel_c,
  xcorr_kernel_c,
  xcorr_kernel_c,
  MAY_HAVE_SSE4_1(xcorr_kernel),
  MAY_HAVE_SSE4_1(xcorr_kernel)
};

#endif

#if (defined(OPUS_X86_MAY_HAVE_SSE4_1) && !defined(OPUS_X86_PRESUME_SSE4_1)) ||  \
 (!defined(OPUS_X86_MAY_HAVE_SSE_4_1) && defined(OPUS_X86_MAY_HAVE_SSE2) && !defined(OPUS_X86_PRESUME_SSE2))

opus_val32 (*const CELT_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *y,
         int              N
) = {
  celt_inner_prod_c,
  celt_inner_prod_c,
  MAY_HAVE_SSE2(celt_inner_prod),
  MAY_HAVE_SSE4_1(celt_inner_prod),
  MAY_HAVE_SSE4_1(celt_inner_prod)
};

#endif

# else

#if defined(OPUS_X86_MAY_HAVE_SSE) && !defined(OPUS_X86_PRESUME_SSE)

void (*const XCORR_KERNEL_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *y,
         opus_val32       sum[4],
         int              len
) = {
  xcorr_kernel_c,
  MAY_HAVE_SSE(xcorr_kernel),
  MAY_HAVE_SSE(xcorr_kernel),
  MAY_HAVE_SSE(xcorr_kernel),
  MAY_HAVE_SSE(xcorr_kernel)
};

opus_val32 (*const CELT_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
         const opus_val16 *x,
         const opus_val16 *y,
         int              N
) = {
  celt_inner_prod_c,
  MAY_HAVE_SSE(celt_inner_prod),
  MAY_HAVE_SSE(celt_inner_prod),
  MAY_HAVE_SSE(celt_inner_prod),
  MAY_HAVE_SSE(celt_inner_prod)
};

void (*const DUAL_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
                    const opus_val16 *x,
                    const opus_val16 *y01,
                    const opus_val16 *y02,
                    int               N,
                    opus_val32       *xy1,
                    opus_val32       *xy2
) = {
  dual_inner_prod_c,
  MAY_HAVE_SSE(dual_inner_prod),
  MAY_HAVE_SSE(dual_inner_prod),
  MAY_HAVE_SSE(dual_inner_prod),
  MAY_HAVE_SSE(dual_inner_prod)
};

void (*const COMB_FILTER_CONST_IMPL[OPUS_ARCHMASK + 1])(
              opus_val32 *y,
              opus_val32 *x,
              int         T,
              int         N,
              opus_val16  g10,
              opus_val16  g11,
              opus_val16  g12
) = {
  comb_filter_const_c,
  MAY_HAVE_SSE(comb_filter_const),
  MAY_HAVE_SSE(comb_filter_const),
  MAY_HAVE_SSE(comb_filter_const),
  MAY_HAVE_SSE(comb_filter_const)
};

#endif

#if defined(OPUS_X86_MAY_HAVE_SSE2) && !defined(OPUS_X86_PRESUME_SSE2)
opus_val16 (*const OP_PVQ_SEARCH_IMPL[OPUS_ARCHMASK + 1])(
      celt_norm *_X, int *iy, int K, int N, int arch
) = {
  op_pvq_search_c,
  op_pvq_search_c,
  MAY_HAVE_SSE2(op_pvq_search),
  MAY_HAVE_SSE2(op_pvq_search),
  MAY_HAVE_SSE2(op_pvq_search)
};
#endif

#endif
#endif
