

#ifndef PITCH_SSE_H
#define PITCH_SSE_H

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)
void xcorr_kernel_sse4_1(
                    const opus_int16 *x,
                    const opus_int16 *y,
                    opus_val32       sum[4],
                    int              len);
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)
void xcorr_kernel_sse(
                    const opus_val16 *x,
                    const opus_val16 *y,
                    opus_val32       sum[4],
                    int              len);
#endif

#if defined(OPUS_X86_PRESUME_SSE4_1) && defined(FIXED_POINT)
#define OVERRIDE_XCORR_KERNEL
#define xcorr_kernel(x, y, sum, len, arch) \
    ((void)arch, xcorr_kernel_sse4_1(x, y, sum, len))

#elif defined(OPUS_X86_PRESUME_SSE) && !defined(FIXED_POINT)
#define OVERRIDE_XCORR_KERNEL
#define xcorr_kernel(x, y, sum, len, arch) \
    ((void)arch, xcorr_kernel_sse(x, y, sum, len))

#elif (defined(OPUS_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)) || (defined(OPUS_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT))

extern void (*const XCORR_KERNEL_IMPL[OPUS_ARCHMASK + 1])(
                    const opus_val16 *x,
                    const opus_val16 *y,
                    opus_val32       sum[4],
                    int              len);

#define OVERRIDE_XCORR_KERNEL
#define xcorr_kernel(x, y, sum, len, arch) \
    ((*XCORR_KERNEL_IMPL[(arch) & OPUS_ARCHMASK])(x, y, sum, len))

#endif

#if defined(OPUS_X86_MAY_HAVE_SSE4_1) && defined(FIXED_POINT)
opus_val32 celt_inner_prod_sse4_1(
    const opus_int16 *x,
    const opus_int16 *y,
    int               N);
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE2) && defined(FIXED_POINT)
opus_val32 celt_inner_prod_sse2(
    const opus_int16 *x,
    const opus_int16 *y,
    int               N);
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)
opus_val32 celt_inner_prod_sse(
    const opus_val16 *x,
    const opus_val16 *y,
    int               N);
#endif

#if defined(OPUS_X86_PRESUME_SSE4_1) && defined(FIXED_POINT)
#define OVERRIDE_CELT_INNER_PROD
#define celt_inner_prod(x, y, N, arch) \
    ((void)arch, celt_inner_prod_sse4_1(x, y, N))

#elif defined(OPUS_X86_PRESUME_SSE2) && defined(FIXED_POINT) && !defined(OPUS_X86_MAY_HAVE_SSE4_1)
#define OVERRIDE_CELT_INNER_PROD
#define celt_inner_prod(x, y, N, arch) \
    ((void)arch, celt_inner_prod_sse2(x, y, N))

#elif defined(OPUS_X86_PRESUME_SSE) && !defined(FIXED_POINT)
#define OVERRIDE_CELT_INNER_PROD
#define celt_inner_prod(x, y, N, arch) \
    ((void)arch, celt_inner_prod_sse(x, y, N))

#elif ((defined(OPUS_X86_MAY_HAVE_SSE4_1) || defined(OPUS_X86_MAY_HAVE_SSE2)) && defined(FIXED_POINT)) || \
    (defined(OPUS_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT))

extern opus_val32 (*const CELT_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
                    const opus_val16 *x,
                    const opus_val16 *y,
                    int               N);

#define OVERRIDE_CELT_INNER_PROD
#define celt_inner_prod(x, y, N, arch) \
    ((*CELT_INNER_PROD_IMPL[(arch) & OPUS_ARCHMASK])(x, y, N))

#endif

#if defined(OPUS_X86_MAY_HAVE_SSE) && !defined(FIXED_POINT)

#define OVERRIDE_DUAL_INNER_PROD
#define OVERRIDE_COMB_FILTER_CONST

#undef dual_inner_prod
#undef comb_filter_const

void dual_inner_prod_sse(const opus_val16 *x,
    const opus_val16 *y01,
    const opus_val16 *y02,
    int               N,
    opus_val32       *xy1,
    opus_val32       *xy2);

void comb_filter_const_sse(opus_val32 *y,
    opus_val32 *x,
    int         T,
    int         N,
    opus_val16  g10,
    opus_val16  g11,
    opus_val16  g12);

#if defined(OPUS_X86_PRESUME_SSE)
# define dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) \
    ((void)(arch),dual_inner_prod_sse(x, y01, y02, N, xy1, xy2))

# define comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
    ((void)(arch),comb_filter_const_sse(y, x, T, N, g10, g11, g12))
#else

extern void (*const DUAL_INNER_PROD_IMPL[OPUS_ARCHMASK + 1])(
              const opus_val16 *x,
              const opus_val16 *y01,
              const opus_val16 *y02,
              int               N,
              opus_val32       *xy1,
              opus_val32       *xy2);

#define dual_inner_prod(x, y01, y02, N, xy1, xy2, arch) \
    ((*DUAL_INNER_PROD_IMPL[(arch) & OPUS_ARCHMASK])(x, y01, y02, N, xy1, xy2))

extern void (*const COMB_FILTER_CONST_IMPL[OPUS_ARCHMASK + 1])(
              opus_val32 *y,
              opus_val32 *x,
              int         T,
              int         N,
              opus_val16  g10,
              opus_val16  g11,
              opus_val16  g12);

#define comb_filter_const(y, x, T, N, g10, g11, g12, arch) \
    ((*COMB_FILTER_CONST_IMPL[(arch) & OPUS_ARCHMASK])(y, x, T, N, g10, g11, g12))

#define NON_STATIC_COMB_FILTER_CONST_C

#endif
#endif

#endif
