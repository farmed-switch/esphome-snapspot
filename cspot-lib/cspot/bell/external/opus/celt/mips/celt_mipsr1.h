

#ifndef __CELT_MIPSR1_H__
#define __CELT_MIPSR1_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define CELT_C

#include "os_support.h"
#include "mdct.h"
#include <math.h>
#include "celt.h"
#include "pitch.h"
#include "bands.h"
#include "modes.h"
#include "entcode.h"
#include "quant_bands.h"
#include "rate.h"
#include "stack_alloc.h"
#include "mathops.h"
#include "float_cast.h"
#include <stdarg.h>
#include "celt_lpc.h"
#include "vq.h"

#define OVERRIDE_COMB_FILTER_CONST
#define OVERRIDE_comb_filter
void comb_filter(opus_val32 *y, opus_val32 *x, int T0, int T1, int N,
      opus_val16 g0, opus_val16 g1, int tapset0, int tapset1,
      const opus_val16 *window, int overlap, int arch)
{
   int i;
   opus_val32 x0, x1, x2, x3, x4;

   (void)arch;

   opus_val16 g00, g01, g02, g10, g11, g12;
   static const opus_val16 gains[3][3] = {
         {QCONST16(0.3066406250f, 15), QCONST16(0.2170410156f, 15), QCONST16(0.1296386719f, 15)},
         {QCONST16(0.4638671875f, 15), QCONST16(0.2680664062f, 15), QCONST16(0.f, 15)},
         {QCONST16(0.7998046875f, 15), QCONST16(0.1000976562f, 15), QCONST16(0.f, 15)}};

   if (g0==0 && g1==0)
   {

      if (x!=y)
         OPUS_MOVE(y, x, N);
      return;
   }

   g00 = MULT16_16_P15(g0, gains[tapset0][0]);
   g01 = MULT16_16_P15(g0, gains[tapset0][1]);
   g02 = MULT16_16_P15(g0, gains[tapset0][2]);
   g10 = MULT16_16_P15(g1, gains[tapset1][0]);
   g11 = MULT16_16_P15(g1, gains[tapset1][1]);
   g12 = MULT16_16_P15(g1, gains[tapset1][2]);
   x1 = x[-T1+1];
   x2 = x[-T1  ];
   x3 = x[-T1-1];
   x4 = x[-T1-2];

   if (g0==g1 && T0==T1 && tapset0==tapset1)
      overlap=0;

   for (i=0;i<overlap;i++)
   {
      opus_val16 f;
      opus_val32 res;
      f = MULT16_16_Q15(window[i],window[i]);
      x0= x[i-T1+2];

      asm volatile("MULT $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15((Q15ONE-f),g00)), "r" ((int)x[i-T0]));

      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15((Q15ONE-f),g01)), "r" ((int)ADD32(x[i-T0-1],x[i-T0+1])));
      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15((Q15ONE-f),g02)), "r" ((int)ADD32(x[i-T0-2],x[i-T0+2])));
      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15(f,g10)), "r" ((int)x2));
      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15(f,g11)), "r" ((int)ADD32(x3,x1)));
      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)MULT16_16_Q15(f,g12)), "r" ((int)ADD32(x4,x0)));

      asm volatile("EXTR.W %0,$ac1, %1" : "=r" (res): "i" (15));

      y[i] = x[i] + res;

      x4=x3;
      x3=x2;
      x2=x1;
      x1=x0;
   }

   x4 = x[i-T1-2];
   x3 = x[i-T1-1];
   x2 = x[i-T1];
   x1 = x[i-T1+1];

   if (g1==0)
   {

      if (x!=y)
         OPUS_MOVE(y+overlap, x+overlap, N-overlap);
      return;
   }

   for (i=overlap;i<N;i++)
   {
      opus_val32 res;
      x0=x[i-T1+2];

      asm volatile("MULT $ac1, %0, %1" : : "r" ((int)g10), "r" ((int)x2));

      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)g11), "r" ((int)ADD32(x3,x1)));
      asm volatile("MADD $ac1, %0, %1" : : "r" ((int)g12), "r" ((int)ADD32(x4,x0)));
      asm volatile("EXTR.W %0,$ac1, %1" : "=r" (res): "i" (15));
      y[i] = x[i] + res;
      x4=x3;
      x3=x2;
      x2=x1;
      x1=x0;
   }
}

#endif
