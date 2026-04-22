

#ifndef __VQ_MIPSR1_H__
#define __VQ_MIPSR1_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mathops.h"
#include "arch.h"

#define OVERRIDE_vq_exp_rotation1
static void exp_rotation1(celt_norm *X, int len, int stride, opus_val16 c, opus_val16 s)
{
   int i;
   opus_val16 ms;
   celt_norm *Xptr;
   Xptr = X;
   ms = NEG16(s);
   for (i=0;i<len-stride;i++)
   {
      celt_norm x1, x2;
      x1 = Xptr[0];
      x2 = Xptr[stride];
      Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
      *Xptr++      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
   }
   Xptr = &X[len-2*stride-1];
   for (i=len-2*stride-1;i>=0;i--)
   {
      celt_norm x1, x2;
      x1 = Xptr[0];
      x2 = Xptr[stride];
      Xptr[stride] = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x2),  s, x1), 15));
      *Xptr--      = EXTRACT16(PSHR32(MAC16_16(MULT16_16(c, x1), ms, x2), 15));
   }
}

#define OVERRIDE_renormalise_vector
void renormalise_vector(celt_norm *X, int N, opus_val16 gain, int arch)
{
   int i;
#ifdef FIXED_POINT
   int k;
#endif
   opus_val32 E = EPSILON;
   opus_val16 g;
   opus_val32 t;
   celt_norm *xptr = X;
   int X0, X1;

   (void)arch;

   asm volatile("mult $ac1, $0, $0");
   asm volatile("MTLO %0, $ac1" : :"r" (E));

   for (i=0;i<N-2;i+=2)
   {
      X0 = (int)*xptr++;
      asm volatile("MADD $ac1, %0, %1" : : "r" (X0), "r" (X0));

      X1 = (int)*xptr++;
      asm volatile("MADD $ac1, %0, %1" : : "r" (X1), "r" (X1));
   }

   for (;i<N;i++)
   {
      X0 = (int)*xptr++;
      asm volatile("MADD $ac1, %0, %1" : : "r" (X0), "r" (X0));
   }

   asm volatile("MFLO %0, $ac1" : "=r" (E));
#ifdef FIXED_POINT
   k = celt_ilog2(E)>>1;
#endif
   t = VSHR32(E, 2*(k-7));
   g = MULT16_16_P15(celt_rsqrt_norm(t),gain);

   xptr = X;
   for (i=0;i<N;i++)
   {
      *xptr = EXTRACT16(PSHR32(MULT16_16(g, *xptr), k+1));
      xptr++;
   }

}

#endif
