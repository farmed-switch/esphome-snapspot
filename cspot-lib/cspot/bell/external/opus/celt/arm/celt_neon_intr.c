

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arm_neon.h>
#include "../pitch.h"

#if defined(FIXED_POINT)
void xcorr_kernel_neon_fixed(const opus_val16 * x, const opus_val16 * y, opus_val32 sum[4], int len)
{
   int j;
   int32x4_t a = vld1q_s32(sum);

   int16x4_t y0 = vld1_s16(y);
   y += 4;

   for (j = 0; j + 8 <= len; j += 8)
   {

      int16x8_t xx = vld1q_s16(x);
      int16x4_t x0 = vget_low_s16(xx);
      int16x4_t x4 = vget_high_s16(xx);

      int16x8_t yy = vld1q_s16(y);
      int16x4_t y4 = vget_low_s16(yy);
      int16x4_t y8 = vget_high_s16(yy);
      int32x4_t a0 = vmlal_lane_s16(a, y0, x0, 0);
      int32x4_t a1 = vmlal_lane_s16(a0, y4, x4, 0);

      int16x4_t y1 = vext_s16(y0, y4, 1);
      int16x4_t y5 = vext_s16(y4, y8, 1);
      int32x4_t a2 = vmlal_lane_s16(a1, y1, x0, 1);
      int32x4_t a3 = vmlal_lane_s16(a2, y5, x4, 1);

      int16x4_t y2 = vext_s16(y0, y4, 2);
      int16x4_t y6 = vext_s16(y4, y8, 2);
      int32x4_t a4 = vmlal_lane_s16(a3, y2, x0, 2);
      int32x4_t a5 = vmlal_lane_s16(a4, y6, x4, 2);

      int16x4_t y3 = vext_s16(y0, y4, 3);
      int16x4_t y7 = vext_s16(y4, y8, 3);
      int32x4_t a6 = vmlal_lane_s16(a5, y3, x0, 3);
      int32x4_t a7 = vmlal_lane_s16(a6, y7, x4, 3);

      y0 = y8;
      a = a7;
      x += 8;
      y += 8;
   }

   for (; j < len; j++)
   {
      int16x4_t x0 = vld1_dup_s16(x);
      int32x4_t a0 = vmlal_s16(a, y0, x0);

      int16x4_t y4 = vld1_dup_s16(y);
      y0 = vext_s16(y0, y4, 1);
      a = a0;
      x++;
      y++;
   }

   vst1q_s32(sum, a);
}

#else

static void xcorr_kernel_neon_float(const float32_t *x, const float32_t *y,
      float32_t sum[4], int len) {
   float32x4_t YY[3];
   float32x4_t YEXT[3];
   float32x4_t XX[2];
   float32x2_t XX_2;
   float32x4_t SUMM;
   const float32_t *xi = x;
   const float32_t *yi = y;

   celt_assert(len>0);

   YY[0] = vld1q_f32(yi);
   SUMM = vdupq_n_f32(0);

   while (len > 8) {
      yi += 4;
      YY[1] = vld1q_f32(yi);
      yi += 4;
      YY[2] = vld1q_f32(yi);

      XX[0] = vld1q_f32(xi);
      xi += 4;
      XX[1] = vld1q_f32(xi);
      xi += 4;

      SUMM = vmlaq_lane_f32(SUMM, YY[0], vget_low_f32(XX[0]), 0);
      YEXT[0] = vextq_f32(YY[0], YY[1], 1);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[0], vget_low_f32(XX[0]), 1);
      YEXT[1] = vextq_f32(YY[0], YY[1], 2);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[1], vget_high_f32(XX[0]), 0);
      YEXT[2] = vextq_f32(YY[0], YY[1], 3);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[2], vget_high_f32(XX[0]), 1);

      SUMM = vmlaq_lane_f32(SUMM, YY[1], vget_low_f32(XX[1]), 0);
      YEXT[0] = vextq_f32(YY[1], YY[2], 1);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[0], vget_low_f32(XX[1]), 1);
      YEXT[1] = vextq_f32(YY[1], YY[2], 2);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[1], vget_high_f32(XX[1]), 0);
      YEXT[2] = vextq_f32(YY[1], YY[2], 3);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[2], vget_high_f32(XX[1]), 1);

      YY[0] = YY[2];
      len -= 8;
   }

   if (len > 4) {
      yi += 4;
      YY[1] = vld1q_f32(yi);

      XX[0] = vld1q_f32(xi);
      xi += 4;

      SUMM = vmlaq_lane_f32(SUMM, YY[0], vget_low_f32(XX[0]), 0);
      YEXT[0] = vextq_f32(YY[0], YY[1], 1);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[0], vget_low_f32(XX[0]), 1);
      YEXT[1] = vextq_f32(YY[0], YY[1], 2);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[1], vget_high_f32(XX[0]), 0);
      YEXT[2] = vextq_f32(YY[0], YY[1], 3);
      SUMM = vmlaq_lane_f32(SUMM, YEXT[2], vget_high_f32(XX[0]), 1);

      YY[0] = YY[1];
      len -= 4;
   }

   while (--len > 0) {
      XX_2 = vld1_dup_f32(xi++);
      SUMM = vmlaq_lane_f32(SUMM, YY[0], XX_2, 0);
      YY[0]= vld1q_f32(++yi);
   }

   XX_2 = vld1_dup_f32(xi);
   SUMM = vmlaq_lane_f32(SUMM, YY[0], XX_2, 0);

   vst1q_f32(sum, SUMM);
}

void celt_pitch_xcorr_float_neon(const opus_val16 *_x, const opus_val16 *_y,
                        opus_val32 *xcorr, int len, int max_pitch, int arch) {
   int i;
   (void)arch;
   celt_assert(max_pitch > 0);
   celt_sig_assert((((unsigned char *)_x-(unsigned char *)NULL)&3)==0);

   for (i = 0; i < (max_pitch-3); i += 4) {
      xcorr_kernel_neon_float((const float32_t *)_x, (const float32_t *)_y+i,
            (float32_t *)xcorr+i, len);
   }

   for (; i < max_pitch; i++) {
      xcorr[i] = celt_inner_prod_neon(_x, _y+i, len);
   }
}
#endif
