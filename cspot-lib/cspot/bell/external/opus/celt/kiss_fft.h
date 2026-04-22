

#ifndef KISS_FFT_H
#define KISS_FFT_H

#include <stdlib.h>
#include <math.h>
#include "arch.h"
#include "cpu_support.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_SIMD
# include <xmmintrin.h>
# define kiss_fft_scalar __m128
#define KISS_FFT_MALLOC(nbytes) memalign(16,nbytes)
#else
#define KISS_FFT_MALLOC opus_alloc
#endif

#ifdef FIXED_POINT
#include "arch.h"

#  define kiss_fft_scalar opus_int32
#  define kiss_twiddle_scalar opus_int16

#else
# ifndef kiss_fft_scalar

#   define kiss_fft_scalar float
#   define kiss_twiddle_scalar float
#   define KF_SUFFIX _celt_single
# endif
#endif

typedef struct {
    kiss_fft_scalar r;
    kiss_fft_scalar i;
}kiss_fft_cpx;

typedef struct {
   kiss_twiddle_scalar r;
   kiss_twiddle_scalar i;
}kiss_twiddle_cpx;

#define MAXFACTORS 8

typedef struct arch_fft_state{
   int is_supported;
   void *priv;
} arch_fft_state;

typedef struct kiss_fft_state{
    int nfft;
    opus_val16 scale;
#ifdef FIXED_POINT
    int scale_shift;
#endif
    int shift;
    opus_int16 factors[2*MAXFACTORS];
    const opus_int16 *bitrev;
    const kiss_twiddle_cpx *twiddles;
    arch_fft_state *arch_fft;
} kiss_fft_state;

#if defined(HAVE_ARM_NE10)
#include "arm/fft_arm.h"
#endif

kiss_fft_state *opus_fft_alloc_twiddles(int nfft,void * mem,size_t * lenmem, const kiss_fft_state *base, int arch);

kiss_fft_state *opus_fft_alloc(int nfft,void * mem,size_t * lenmem, int arch);

void opus_fft_c(const kiss_fft_state *cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);
void opus_ifft_c(const kiss_fft_state *cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout);

void opus_fft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout);
void opus_ifft_impl(const kiss_fft_state *st,kiss_fft_cpx *fout);

void opus_fft_free(const kiss_fft_state *cfg, int arch);

void opus_fft_free_arch_c(kiss_fft_state *st);
int opus_fft_alloc_arch_c(kiss_fft_state *st);

#if !defined(OVERRIDE_OPUS_FFT)

#if defined(OPUS_HAVE_RTCD) && (defined(HAVE_ARM_NE10))

extern int (*const OPUS_FFT_ALLOC_ARCH_IMPL[OPUS_ARCHMASK+1])(
 kiss_fft_state *st);

#define opus_fft_alloc_arch(_st, arch) \
         ((*OPUS_FFT_ALLOC_ARCH_IMPL[(arch)&OPUS_ARCHMASK])(_st))

extern void (*const OPUS_FFT_FREE_ARCH_IMPL[OPUS_ARCHMASK+1])(
 kiss_fft_state *st);
#define opus_fft_free_arch(_st, arch) \
         ((*OPUS_FFT_FREE_ARCH_IMPL[(arch)&OPUS_ARCHMASK])(_st))

extern void (*const OPUS_FFT[OPUS_ARCHMASK+1])(const kiss_fft_state *cfg,
 const kiss_fft_cpx *fin, kiss_fft_cpx *fout);
#define opus_fft(_cfg, _fin, _fout, arch) \
   ((*OPUS_FFT[(arch)&OPUS_ARCHMASK])(_cfg, _fin, _fout))

extern void (*const OPUS_IFFT[OPUS_ARCHMASK+1])(const kiss_fft_state *cfg,
 const kiss_fft_cpx *fin, kiss_fft_cpx *fout);
#define opus_ifft(_cfg, _fin, _fout, arch) \
   ((*OPUS_IFFT[(arch)&OPUS_ARCHMASK])(_cfg, _fin, _fout))

#else

#define opus_fft_alloc_arch(_st, arch) \
         ((void)(arch), opus_fft_alloc_arch_c(_st))

#define opus_fft_free_arch(_st, arch) \
         ((void)(arch), opus_fft_free_arch_c(_st))

#define opus_fft(_cfg, _fin, _fout, arch) \
         ((void)(arch), opus_fft_c(_cfg, _fin, _fout))

#define opus_ifft(_cfg, _fin, _fout, arch) \
         ((void)(arch), opus_ifft_c(_cfg, _fin, _fout))

#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
