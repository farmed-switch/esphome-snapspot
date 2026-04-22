

#if !defined(X86CPU_H)
# define X86CPU_H

# if defined(OPUS_X86_MAY_HAVE_SSE)
#  define MAY_HAVE_SSE(name) name ## _sse
# else
#  define MAY_HAVE_SSE(name) name ## _c
# endif

# if defined(OPUS_X86_MAY_HAVE_SSE2)
#  define MAY_HAVE_SSE2(name) name ## _sse2
# else
#  define MAY_HAVE_SSE2(name) name ## _c
# endif

# if defined(OPUS_X86_MAY_HAVE_SSE4_1)
#  define MAY_HAVE_SSE4_1(name) name ## _sse4_1
# else
#  define MAY_HAVE_SSE4_1(name) name ## _c
# endif

# if defined(OPUS_X86_MAY_HAVE_AVX)
#  define MAY_HAVE_AVX(name) name ## _avx
# else
#  define MAY_HAVE_AVX(name) name ## _c
# endif

# if defined(OPUS_HAVE_RTCD)
int opus_select_arch(void);
# endif

#define OP_CVTEPI8_EPI32_M32(x) \
 (_mm_cvtepi8_epi32(_mm_cvtsi32_si128(*(int *)(x))))

#define OP_CVTEPI16_EPI32_M64(x) \
 (_mm_cvtepi16_epi32(_mm_loadl_epi64((__m128i *)(x))))

#endif
