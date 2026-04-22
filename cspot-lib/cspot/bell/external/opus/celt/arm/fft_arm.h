

#if !defined(FFT_ARM_H)
#define FFT_ARM_H

#include "kiss_fft.h"

#if defined(HAVE_ARM_NE10)

int opus_fft_alloc_arm_neon(kiss_fft_state *st);
void opus_fft_free_arm_neon(kiss_fft_state *st);

void opus_fft_neon(const kiss_fft_state *st,
                   const kiss_fft_cpx *fin,
                   kiss_fft_cpx *fout);

void opus_ifft_neon(const kiss_fft_state *st,
                    const kiss_fft_cpx *fin,
                    kiss_fft_cpx *fout);

#if !defined(OPUS_HAVE_RTCD)
#define OVERRIDE_OPUS_FFT (1)

#define opus_fft_alloc_arch(_st, arch) \
   ((void)(arch), opus_fft_alloc_arm_neon(_st))

#define opus_fft_free_arch(_st, arch) \
   ((void)(arch), opus_fft_free_arm_neon(_st))

#define opus_fft(_st, _fin, _fout, arch) \
   ((void)(arch), opus_fft_neon(_st, _fin, _fout))

#define opus_ifft(_st, _fin, _fout, arch) \
   ((void)(arch), opus_ifft_neon(_st, _fin, _fout))

#endif

#endif

#endif
