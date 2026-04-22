

#ifndef VQ_SSE_H
#define VQ_SSE_H

#if defined(OPUS_X86_MAY_HAVE_SSE2) && !defined(FIXED_POINT)
#define OVERRIDE_OP_PVQ_SEARCH

opus_val16 op_pvq_search_sse2(celt_norm *_X, int *iy, int K, int N, int arch);

#if defined(OPUS_X86_PRESUME_SSE2)
#define op_pvq_search(x, iy, K, N, arch) \
    (op_pvq_search_sse2(x, iy, K, N, arch))

#else

extern opus_val16 (*const OP_PVQ_SEARCH_IMPL[OPUS_ARCHMASK + 1])(
      celt_norm *_X, int *iy, int K, int N, int arch);

#  define op_pvq_search(X, iy, K, N, arch) \
    ((*OP_PVQ_SEARCH_IMPL[(arch) & OPUS_ARCHMASK])(X, iy, K, N, arch))

#endif
#endif

#endif
