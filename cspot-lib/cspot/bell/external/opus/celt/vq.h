

#ifndef VQ_H
#define VQ_H

#include "entenc.h"
#include "entdec.h"
#include "modes.h"

#if (defined(OPUS_X86_MAY_HAVE_SSE2) && !defined(FIXED_POINT))
#include "x86/vq_sse.h"
#endif

void exp_rotation(celt_norm *X, int len, int dir, int stride, int K, int spread);

opus_val16 op_pvq_search_c(celt_norm *X, int *iy, int K, int N, int arch);

#if !defined(OVERRIDE_OP_PVQ_SEARCH)
#define op_pvq_search(x, iy, K, N, arch) \
    (op_pvq_search_c(x, iy, K, N, arch))
#endif

unsigned alg_quant(celt_norm *X, int N, int K, int spread, int B, ec_enc *enc,
      opus_val16 gain, int resynth, int arch);

unsigned alg_unquant(celt_norm *X, int N, int K, int spread, int B,
      ec_dec *dec, opus_val16 gain);

void renormalise_vector(celt_norm *X, int N, opus_val16 gain, int arch);

int stereo_itheta(const celt_norm *X, const celt_norm *Y, int stereo, int N, int arch);

#endif
