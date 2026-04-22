

#ifndef SKIP_CONFIG_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#endif

#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#include "mdct.h"
#include "stack_alloc.h"

void clt_mdct_forward_neon(const mdct_lookup *l,
                           kiss_fft_scalar *in,
                           kiss_fft_scalar * OPUS_RESTRICT out,
                           const opus_val16 *window,
                           int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   VARDECL(kiss_fft_scalar, f);
   VARDECL(kiss_fft_cpx, f2);
   const kiss_fft_state *st = l->kfft[shift];
   const kiss_twiddle_scalar *trig;

   SAVE_STACK;

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

         *yp++ = MULT16_32_Q15(*wp2, xp1[N2]) + MULT16_32_Q15(*wp1,*xp2);
         *yp++ = MULT16_32_Q15(*wp1, *xp1)    - MULT16_32_Q15(*wp2, xp2[-N2]);
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

         *yp++ =  -MULT16_32_Q15(*wp1, xp1[-N2]) + MULT16_32_Q15(*wp2, *xp2);
         *yp++ = MULT16_32_Q15(*wp2, *xp1)     + MULT16_32_Q15(*wp1, xp2[N2]);
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
         yr = S_MUL(re,t0)  -  S_MUL(im,t1);
         yi = S_MUL(im,t0)  +  S_MUL(re,t1);
         yc.r = yr;
         yc.i = yi;
         f2[i] = yc;
      }
   }

   opus_fft(st, f2, (kiss_fft_cpx *)f, arch);

   {

      const kiss_fft_cpx * OPUS_RESTRICT fp = (kiss_fft_cpx *)f;
      kiss_fft_scalar * OPUS_RESTRICT yp1 = out;
      kiss_fft_scalar * OPUS_RESTRICT yp2 = out+stride*(N2-1);
      const kiss_twiddle_scalar *t = &trig[0];

      for(i=0;i<N4;i++)
      {
         kiss_fft_scalar yr, yi;
         yr = S_MUL(fp->i,t[N4+i]) - S_MUL(fp->r,t[i]);
         yi = S_MUL(fp->r,t[N4+i]) + S_MUL(fp->i,t[i]);
         *yp1 = yr;
         *yp2 = yi;
         fp++;
         yp1 += 2*stride;
         yp2 -= 2*stride;
      }
   }
   RESTORE_STACK;
}

void clt_mdct_backward_neon(const mdct_lookup *l,
                            kiss_fft_scalar *in,
                            kiss_fft_scalar * OPUS_RESTRICT out,
                            const opus_val16 * OPUS_RESTRICT window,
                            int overlap, int shift, int stride, int arch)
{
   int i;
   int N, N2, N4;
   VARDECL(kiss_fft_scalar, f);
   const kiss_twiddle_scalar *trig;
   const kiss_fft_state *st = l->kfft[shift];

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

   {

      const kiss_fft_scalar * OPUS_RESTRICT xp1 = in;
      const kiss_fft_scalar * OPUS_RESTRICT xp2 = in+stride*(N2-1);
      kiss_fft_scalar * OPUS_RESTRICT yp = f;
      const kiss_twiddle_scalar * OPUS_RESTRICT t = &trig[0];
      for(i=0;i<N4;i++)
      {
         kiss_fft_scalar yr, yi;
         yr = S_MUL(*xp2, t[i]) + S_MUL(*xp1, t[N4+i]);
         yi = S_MUL(*xp1, t[i]) - S_MUL(*xp2, t[N4+i]);
         yp[2*i] = yr;
         yp[2*i+1] = yi;
         xp1+=2*stride;
         xp2-=2*stride;
      }
   }

   opus_ifft(st, (kiss_fft_cpx *)f, (kiss_fft_cpx*)(out+(overlap>>1)), arch);

   {
      kiss_fft_scalar * yp0 = out+(overlap>>1);
      kiss_fft_scalar * yp1 = out+(overlap>>1)+N2-2;
      const kiss_twiddle_scalar *t = &trig[0];

      for(i=0;i<(N4+1)>>1;i++)
      {
         kiss_fft_scalar re, im, yr, yi;
         kiss_twiddle_scalar t0, t1;
         re = yp0[0];
         im = yp0[1];
         t0 = t[i];
         t1 = t[N4+i];

         yr = S_MUL(re,t0) + S_MUL(im,t1);
         yi = S_MUL(re,t1) - S_MUL(im,t0);
         re = yp1[0];
         im = yp1[1];
         yp0[0] = yr;
         yp1[1] = yi;

         t0 = t[(N4-i-1)];
         t1 = t[(N2-i-1)];

         yr = S_MUL(re,t0) + S_MUL(im,t1);
         yi = S_MUL(re,t1) - S_MUL(im,t0);
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
   RESTORE_STACK;
}
