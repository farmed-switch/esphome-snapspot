

#ifndef __MDCT_MIPSR1_H__
#define __MDCT_MIPSR1_H__

#ifndef SKIP_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#include "mdct.h"
#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#include <math.h>
#include "os_support.h"
#include "mathops.h"
#include "stack_alloc.h"

#define OVERRIDE_clt_mdct_forward
void clt_mdct_forward(const mdct_lookup *l, kiss_fft_scalar *in, kiss_fft_scalar * OPUS_RESTRICT out,
      const opus_val16 *window, int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   VARDECL(kiss_fft_scalar, f);
   VARDECL(kiss_fft_cpx, f2);
   const kiss_fft_state *st = l->kfft[shift];
   const kiss_twiddle_scalar *trig;
   opus_val16 scale;
#ifdef FIXED_POINT

   int scale_shift = st->scale_shift-1;
#endif

    (void)arch;

   SAVE_STACK;
   scale = st->scale;

   N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   ALLOC(f, N2, kiss_fft_scalar);
   ALLOC(f2, N4, kiss_fft_cpx);

   {

      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in+(overlap>>1);
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+N2-1+(overlap>>1);
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const opus_val16 * OPUS_RESTRICT wp1 = window+(overlap>>1);
      const opus_val16 * OPUS_RESTRICT wp2 = window+(overlap>>1)-1;
      for(i=0;i<((overlap+3)>>2);i++)
      {

          *yp++ = S_MUL_ADD(*wp2, xp1[N2],*wp1,*xp2);
          *yp++ = S_MUL_SUB(*wp1, *xp1,*wp2, xp2[-N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
      wp1 = window;
      wp2 = window+overlap-1;
      for(;i<N4-((overlap+3)>>2);i++)
      {

         *yp++ = *xp2;
         *yp++ = *xp1;
         xp1+=2;
         xp2-=2;
      }
      for(;i<N4;i++)
      {

          *yp++ =  S_MUL_SUB(*wp2, *xp2, *wp1, xp1[-N2]);
          *yp++ = S_MUL_ADD(*wp2, *xp1, *wp1, xp2[N2]);
         xp1+=2;
         xp2-=2;
         wp1+=2;
         wp2-=2;
      }
   }

   {
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const kiss_twiddle_scalar *t = &trig[0];
      for(i=0;i<N4;i++)
      {
         kiss_fft_cpx yc;
         kiss_twiddle_scalar t0, t1;
         kiss_fft_scalar re, im, yr, yi;
         t0 = t[i];
         t1 = t[N4+i];
         re = *yp++;
         im = *yp++;

         yr = S_MUL_SUB(re,t0,im,t1);
         yi = S_MUL_ADD(im,t0,re,t1);

         yc.r = yr;
         yc.i = yi;
         yc.r = PSHR32(MULT16_32_Q16(scale, yc.r), scale_shift);
         yc.i = PSHR32(MULT16_32_Q16(scale, yc.i), scale_shift);
         f2[st->bitrev[i]] = yc;
      }
   }

   opus_fft_impl(st, f2);

   {

      const kiss_fft_cpx * OPUS_RESTRICT fp = f2;
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out;
      kiss_fft_scalar * OPUS_RESTRICT yp2 = out+stride*(N2-1);
      const kiss_twiddle_scalar *t = &trig[0];

      for(i=0;i<N4;i++)
      {
         kiss_fft_scalar yr, yi;
         yr = S_MUL_SUB(fp->i,t[N4+i] , fp->r,t[i]);
         yi = S_MUL_ADD(fp->r,t[N4+i] ,fp->i,t[i]);
         *yp1 = yr;
         *yp2 = yi;
         fp++;
         yp1 += 2*stride;
         yp2 -= 2*stride;
      }
   }
   RESTORE_STACK;
}

#define OVERRIDE_clt_mdct_backward
void clt_mdct_backward(const mdct_lookup *l, kiss_fft_scalar *in, kiss_fft_scalar * OPUS_RESTRICT out,
      const opus_val16 * OPUS_RESTRICT window, int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   const kiss_twiddle_scalar *trig;

    (void)arch;

   N = l->n;
   trig = l->trig;
   for (i=0;i<shift;i++)
   {
      N >>= 1;
      trig += N;
   }
   N2 = N>>1;
   N4 = N>>2;

   {

      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in;
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+stride*(N2-1);
      kiss_fft_scalar * OPUS_RESTRICT yp = out+(overlap>>1);
      const kiss_twiddle_scalar * OPUS_RESTRICT t = &trig[0];
      const opus_int16 * OPUS_RESTRICT bitrev = l->kfft[shift]->bitrev;
      for(i=0;i<N4;i++)
      {
         int rev;
         kiss_fft_scalar yr, yi;
         rev = *bitrev++;
         yr = S_MUL_ADD(*xp2, t[i] , *xp1, t[N4+i]);
         yi = S_MUL_SUB(*xp1, t[i] , *xp2, t[N4+i]);

         yp[2*rev+1] = yr;
         yp[2*rev] = yi;

         xp1+=2*stride;
         xp2-=2*stride;
      }
   }

   opus_fft_impl(l->kfft[shift], (kiss_fft_cpx*)(out+(overlap>>1)));

   {
      kiss_fft_scalar * OPUS_RESTRICT yp0 = out+(overlap>>1);
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out+(overlap>>1)+N2-2;
      const kiss_twiddle_scalar *t = &trig[0];

      for(i=0;i<(N4+1)>>1;i++)
      {
         kiss_fft_scalar re, im, yr, yi;
         kiss_twiddle_scalar t0, t1;

         re = yp0[1];
         im = yp0[0];
         t0 = t[i];
         t1 = t[N4+i];

         yr = S_MUL_ADD(re,t0 , im,t1);
         yi = S_MUL_SUB(re,t1 , im,t0);

         re = yp1[1];
         im = yp1[0];
         yp0[0] = yr;
         yp1[1] = yi;

         t0 = t[(N4-i-1)];
         t1 = t[(N2-i-1)];

         yr = S_MUL_ADD(re,t0,im,t1);
         yi = S_MUL_SUB(re,t1,im,t0);
         yp1[0] = yr;
         yp0[1] = yi;
         yp0 += 2;
         yp1 -= 2;
      }
   }

   {
      kiss_fft_scalar * OPUS_RESTRICT xp1 = out+overlap-1;
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out;
      const opus_val16 * OPUS_RESTRICT wp1 = window;
      const opus_val16 * OPUS_RESTRICT wp2 = window+overlap-1;

      for(i = 0; i < overlap/2; i++)
      {
         kiss_fft_scalar x1, x2;
         x1 = *xp1;
         x2 = *yp1;
         *yp1++ = MULT16_32_Q15(*wp2, x2) - MULT16_32_Q15(*wp1, x1);
         *xp1-- = MULT16_32_Q15(*wp1, x2) + MULT16_32_Q15(*wp2, x1);
         wp1++;
         wp2--;
      }
   }
}
#endif
