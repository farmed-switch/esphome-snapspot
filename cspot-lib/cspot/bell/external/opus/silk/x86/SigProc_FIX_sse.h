

#ifndef SIGPROC_FIX_SSE_H
#define SIGPROC_FIX_SSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(OPUS_X86_MAY_HAVE_SSE4_1)
void silk_burg_modified_sse4_1(
    opus_int32                  *res_nrg,
    opus_int                    *res_nrg_Q,
    opus_int32                  A_Q16[],
    const opus_int16            x[],
    const opus_int32            minInvGain_Q30,
    const opus_int              subfr_length,
    const opus_int              nb_subfr,
    const opus_int              D,
    int                         arch
);

#if defined(OPUS_X86_PRESUME_SSE4_1)
#define silk_burg_modified(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch) \
    ((void)(arch), silk_burg_modified_sse4_1(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch))

#else

extern void (*const SILK_BURG_MODIFIED_IMPL[OPUS_ARCHMASK + 1])(
    opus_int32                  *res_nrg,
    opus_int                    *res_nrg_Q,
    opus_int32                  A_Q16[],
    const opus_int16            x[],
    const opus_int32            minInvGain_Q30,
    const opus_int              subfr_length,
    const opus_int              nb_subfr,
    const opus_int              D,
    int                         arch                 );

#  define silk_burg_modified(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch) \
    ((*SILK_BURG_MODIFIED_IMPL[(arch) & OPUS_ARCHMASK])(res_nrg, res_nrg_Q, A_Q16, x, minInvGain_Q30, subfr_length, nb_subfr, D, arch))

#endif

opus_int64 silk_inner_prod16_aligned_64_sse4_1(
    const opus_int16 *inVec1,
    const opus_int16 *inVec2,
    const opus_int   len
);

#if defined(OPUS_X86_PRESUME_SSE4_1)

#define silk_inner_prod16_aligned_64(inVec1, inVec2, len, arch) \
    ((void)(arch),silk_inner_prod16_aligned_64_sse4_1(inVec1, inVec2, len))

#else

extern opus_int64 (*const SILK_INNER_PROD16_ALIGNED_64_IMPL[OPUS_ARCHMASK + 1])(
                    const opus_int16 *inVec1,
                    const opus_int16 *inVec2,
                    const opus_int   len);

#  define silk_inner_prod16_aligned_64(inVec1, inVec2, len, arch) \
    ((*SILK_INNER_PROD16_ALIGNED_64_IMPL[(arch) & OPUS_ARCHMASK])(inVec1, inVec2, len))

#endif
#endif
#endif
