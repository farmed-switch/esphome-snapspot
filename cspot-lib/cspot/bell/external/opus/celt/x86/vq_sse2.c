

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xmmintrin.h>
#include <emmintrin.h>
#include "celt_lpc.h"
#include "stack_alloc.h"
#include "mathops.h"
#include "vq.h"
#include "x86cpu.h"

#ifndef FIXED_POINT

opus_val16 op_pvq_search_sse2(celt_norm *_X, int *iy, int K, int N, int arch)
{
   int i, j;
   int pulsesLeft;
   float xy, yy;
   VARDECL(celt_norm, y);
   VARDECL(celt_norm, X);
   VARDECL(float, signy);
   __m128 signmask;
   __m128 sums;
   __m128i fours;
   SAVE_STACK;

   (void)arch;

   signmask = _mm_set_ps1(-0.f);
   fours = _mm_set_epi32(4, 4, 4, 4);
   ALLOC(y, N+3, celt_norm);
   ALLOC(X, N+3, celt_norm);
   ALLOC(signy, N+3, float);

   OPUS_COPY(X, _X, N);
   X[N] = X[N+1] = X[N+2] = 0;
   sums = _mm_setzero_ps();
   for (j=0;j<N;j+=4)
   {
      __m128 x4, s4;
      x4 = _mm_loadu_ps(&X[j]);
      s4 = _mm_cmplt_ps(x4, _mm_setzero_ps());

      x4 = _mm_andnot_ps(signmask, x4);
      sums = _mm_add_ps(sums, x4);

      _mm_storeu_ps(&y[j], _mm_setzero_ps());
      _mm_storeu_si128((__m128i*)&iy[j], _mm_setzero_si128());
      _mm_storeu_ps(&X[j], x4);
      _mm_storeu_ps(&signy[j], s4);
   }
   sums = _mm_add_ps(sums, _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(1, 0, 3, 2)));
   sums = _mm_add_ps(sums, _mm_shuffle_ps(sums, sums, _MM_SHUFFLE(2, 3, 0, 1)));

   xy = yy = 0;

   pulsesLeft = K;

   if (K > (N>>1))
   {
      __m128i pulses_sum;
      __m128 yy4, xy4;
      __m128 rcp4;
      opus_val32 sum = _mm_cvtss_f32(sums);

      if (!(sum > EPSILON && sum < 64))
      {
         X[0] = QCONST16(1.f,14);
         j=1; do
            X[j]=0;
         while (++j<N);
         sums = _mm_set_ps1(1.f);
      }

      rcp4 = _mm_mul_ps(_mm_set_ps1((float)(K+.8)), _mm_rcp_ps(sums));
      xy4 = yy4 = _mm_setzero_ps();
      pulses_sum = _mm_setzero_si128();
      for (j=0;j<N;j+=4)
      {
         __m128 rx4, x4, y4;
         __m128i iy4;
         x4 = _mm_loadu_ps(&X[j]);
         rx4 = _mm_mul_ps(x4, rcp4);
         iy4 = _mm_cvttps_epi32(rx4);
         pulses_sum = _mm_add_epi32(pulses_sum, iy4);
         _mm_storeu_si128((__m128i*)&iy[j], iy4);
         y4 = _mm_cvtepi32_ps(iy4);
         xy4 = _mm_add_ps(xy4, _mm_mul_ps(x4, y4));
         yy4 = _mm_add_ps(yy4, _mm_mul_ps(y4, y4));

         _mm_storeu_ps(&y[j], _mm_add_ps(y4, y4));
      }
      pulses_sum = _mm_add_epi32(pulses_sum, _mm_shuffle_epi32(pulses_sum, _MM_SHUFFLE(1, 0, 3, 2)));
      pulses_sum = _mm_add_epi32(pulses_sum, _mm_shuffle_epi32(pulses_sum, _MM_SHUFFLE(2, 3, 0, 1)));
      pulsesLeft -= _mm_cvtsi128_si32(pulses_sum);
      xy4 = _mm_add_ps(xy4, _mm_shuffle_ps(xy4, xy4, _MM_SHUFFLE(1, 0, 3, 2)));
      xy4 = _mm_add_ps(xy4, _mm_shuffle_ps(xy4, xy4, _MM_SHUFFLE(2, 3, 0, 1)));
      xy = _mm_cvtss_f32(xy4);
      yy4 = _mm_add_ps(yy4, _mm_shuffle_ps(yy4, yy4, _MM_SHUFFLE(1, 0, 3, 2)));
      yy4 = _mm_add_ps(yy4, _mm_shuffle_ps(yy4, yy4, _MM_SHUFFLE(2, 3, 0, 1)));
      yy = _mm_cvtss_f32(yy4);
   }
   X[N] = X[N+1] = X[N+2] = -100;
   y[N] = y[N+1] = y[N+2] = 100;
   celt_sig_assert(pulsesLeft>=0);

   if (pulsesLeft > N+3)
   {
      opus_val16 tmp = (opus_val16)pulsesLeft;
      yy = MAC16_16(yy, tmp, tmp);
      yy = MAC16_16(yy, tmp, y[0]);
      iy[0] += pulsesLeft;
      pulsesLeft=0;
   }

   for (i=0;i<pulsesLeft;i++)
   {
      int best_id;
      __m128 xy4, yy4;
      __m128 max, max2;
      __m128i count;
      __m128i pos;

      yy = ADD16(yy, 1);
      xy4 = _mm_load1_ps(&xy);
      yy4 = _mm_load1_ps(&yy);
      max = _mm_setzero_ps();
      pos = _mm_setzero_si128();
      count = _mm_set_epi32(3, 2, 1, 0);
      for (j=0;j<N;j+=4)
      {
         __m128 x4, y4, r4;
         x4 = _mm_loadu_ps(&X[j]);
         y4 = _mm_loadu_ps(&y[j]);
         x4 = _mm_add_ps(x4, xy4);
         y4 = _mm_add_ps(y4, yy4);
         y4 = _mm_rsqrt_ps(y4);
         r4 = _mm_mul_ps(x4, y4);

         pos = _mm_max_epi16(pos, _mm_and_si128(count, _mm_castps_si128(_mm_cmpgt_ps(r4, max))));

         max = _mm_max_ps(max, r4);

         count = _mm_add_epi32(count, fours);
      }

      max2 = _mm_max_ps(max, _mm_shuffle_ps(max, max, _MM_SHUFFLE(1, 0, 3, 2)));
      max2 = _mm_max_ps(max2, _mm_shuffle_ps(max2, max2, _MM_SHUFFLE(2, 3, 0, 1)));

      pos = _mm_and_si128(pos, _mm_castps_si128(_mm_cmpeq_ps(max, max2)));
      pos = _mm_max_epi16(pos, _mm_unpackhi_epi64(pos, pos));
      pos = _mm_max_epi16(pos, _mm_shufflelo_epi16(pos, _MM_SHUFFLE(1, 0, 3, 2)));
      best_id = _mm_cvtsi128_si32(pos);

      xy = ADD32(xy, EXTEND32(X[best_id]));

      yy = ADD16(yy, y[best_id]);

      y[best_id] += 2;
      iy[best_id]++;
   }

   for (j=0;j<N;j+=4)
   {
      __m128i y4;
      __m128i s4;
      y4 = _mm_loadu_si128((__m128i*)&iy[j]);
      s4 = _mm_castps_si128(_mm_loadu_ps(&signy[j]));
      y4 = _mm_xor_si128(_mm_add_epi32(y4, s4), s4);
      _mm_storeu_si128((__m128i*)&iy[j], y4);
   }
   RESTORE_STACK;
   return yy;
}

#endif
