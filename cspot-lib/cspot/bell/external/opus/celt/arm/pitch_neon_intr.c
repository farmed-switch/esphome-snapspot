

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <arm_neon.h>
#include "pitch.h"

#ifdef FIXED_POINT

opus_val32 celt_inner_prod_neon(const opus_val16 *x, const opus_val16 *y, int N)
{
    int i;
    opus_val32 xy;
    int16x8_t x_s16x8, y_s16x8;
    int32x4_t xy_s32x4 = vdupq_n_s32(0);
    int64x2_t xy_s64x2;
    int64x1_t xy_s64x1;

    for (i = 0; i < N - 7; i += 8) {
        x_s16x8  = vld1q_s16(&x[i]);
        y_s16x8  = vld1q_s16(&y[i]);
        xy_s32x4 = vmlal_s16(xy_s32x4, vget_low_s16 (x_s16x8), vget_low_s16 (y_s16x8));
        xy_s32x4 = vmlal_s16(xy_s32x4, vget_high_s16(x_s16x8), vget_high_s16(y_s16x8));
    }

    if (N - i >= 4) {
        const int16x4_t x_s16x4 = vld1_s16(&x[i]);
        const int16x4_t y_s16x4 = vld1_s16(&y[i]);
        xy_s32x4 = vmlal_s16(xy_s32x4, x_s16x4, y_s16x4);
        i += 4;
    }

    xy_s64x2 = vpaddlq_s32(xy_s32x4);
    xy_s64x1 = vadd_s64(vget_low_s64(xy_s64x2), vget_high_s64(xy_s64x2));
    xy       = vget_lane_s32(vreinterpret_s32_s64(xy_s64x1), 0);

    for (; i < N; i++) {
        xy = MAC16_16(xy, x[i], y[i]);
    }

#ifdef OPUS_CHECK_ASM
    celt_assert(celt_inner_prod_c(x, y, N) == xy);
#endif

    return xy;
}

void dual_inner_prod_neon(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
        int N, opus_val32 *xy1, opus_val32 *xy2)
{
    int i;
    opus_val32 xy01, xy02;
    int16x8_t x_s16x8, y01_s16x8, y02_s16x8;
    int32x4_t xy01_s32x4 = vdupq_n_s32(0);
    int32x4_t xy02_s32x4 = vdupq_n_s32(0);
    int64x2_t xy01_s64x2, xy02_s64x2;
    int64x1_t xy01_s64x1, xy02_s64x1;

    for (i = 0; i < N - 7; i += 8) {
        x_s16x8    = vld1q_s16(&x[i]);
        y01_s16x8  = vld1q_s16(&y01[i]);
        y02_s16x8  = vld1q_s16(&y02[i]);
        xy01_s32x4 = vmlal_s16(xy01_s32x4, vget_low_s16 (x_s16x8), vget_low_s16 (y01_s16x8));
        xy02_s32x4 = vmlal_s16(xy02_s32x4, vget_low_s16 (x_s16x8), vget_low_s16 (y02_s16x8));
        xy01_s32x4 = vmlal_s16(xy01_s32x4, vget_high_s16(x_s16x8), vget_high_s16(y01_s16x8));
        xy02_s32x4 = vmlal_s16(xy02_s32x4, vget_high_s16(x_s16x8), vget_high_s16(y02_s16x8));
    }

    if (N - i >= 4) {
        const int16x4_t x_s16x4   = vld1_s16(&x[i]);
        const int16x4_t y01_s16x4 = vld1_s16(&y01[i]);
        const int16x4_t y02_s16x4 = vld1_s16(&y02[i]);
        xy01_s32x4 = vmlal_s16(xy01_s32x4, x_s16x4, y01_s16x4);
        xy02_s32x4 = vmlal_s16(xy02_s32x4, x_s16x4, y02_s16x4);
        i += 4;
    }

    xy01_s64x2 = vpaddlq_s32(xy01_s32x4);
    xy02_s64x2 = vpaddlq_s32(xy02_s32x4);
    xy01_s64x1 = vadd_s64(vget_low_s64(xy01_s64x2), vget_high_s64(xy01_s64x2));
    xy02_s64x1 = vadd_s64(vget_low_s64(xy02_s64x2), vget_high_s64(xy02_s64x2));
    xy01       = vget_lane_s32(vreinterpret_s32_s64(xy01_s64x1), 0);
    xy02       = vget_lane_s32(vreinterpret_s32_s64(xy02_s64x1), 0);

    for (; i < N; i++) {
        xy01 = MAC16_16(xy01, x[i], y01[i]);
        xy02 = MAC16_16(xy02, x[i], y02[i]);
    }
    *xy1 = xy01;
    *xy2 = xy02;

#ifdef OPUS_CHECK_ASM
    {
        opus_val32 xy1_c, xy2_c;
        dual_inner_prod_c(x, y01, y02, N, &xy1_c, &xy2_c);
        celt_assert(xy1_c == *xy1);
        celt_assert(xy2_c == *xy2);
    }
#endif
}

#else

#ifdef OPUS_CHECK_ASM

static opus_val32 celt_inner_prod_neon_float_c_simulation(const opus_val16 *x, const opus_val16 *y, int N)
{
   int i;
   opus_val32 xy, xy0 = 0, xy1 = 0, xy2 = 0, xy3 = 0;
   for (i = 0; i < N - 3; i += 4) {
      xy0 = MAC16_16(xy0, x[i + 0], y[i + 0]);
      xy1 = MAC16_16(xy1, x[i + 1], y[i + 1]);
      xy2 = MAC16_16(xy2, x[i + 2], y[i + 2]);
      xy3 = MAC16_16(xy3, x[i + 3], y[i + 3]);
   }
   xy0 += xy2;
   xy1 += xy3;
   xy = xy0 + xy1;
   for (; i < N; i++) {
      xy = MAC16_16(xy, x[i], y[i]);
   }
   return xy;
}

static void dual_inner_prod_neon_float_c_simulation(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
      int N, opus_val32 *xy1, opus_val32 *xy2)
{
   int i;
   opus_val32 xy01, xy02, xy01_0 = 0, xy01_1 = 0, xy01_2 = 0, xy01_3 = 0, xy02_0 = 0, xy02_1 = 0, xy02_2 = 0, xy02_3 = 0;
   for (i = 0; i < N - 3; i += 4) {
      xy01_0 = MAC16_16(xy01_0, x[i + 0], y01[i + 0]);
      xy01_1 = MAC16_16(xy01_1, x[i + 1], y01[i + 1]);
      xy01_2 = MAC16_16(xy01_2, x[i + 2], y01[i + 2]);
      xy01_3 = MAC16_16(xy01_3, x[i + 3], y01[i + 3]);
      xy02_0 = MAC16_16(xy02_0, x[i + 0], y02[i + 0]);
      xy02_1 = MAC16_16(xy02_1, x[i + 1], y02[i + 1]);
      xy02_2 = MAC16_16(xy02_2, x[i + 2], y02[i + 2]);
      xy02_3 = MAC16_16(xy02_3, x[i + 3], y02[i + 3]);
   }
   xy01_0 += xy01_2;
   xy02_0 += xy02_2;
   xy01_1 += xy01_3;
   xy02_1 += xy02_3;
   xy01 = xy01_0 + xy01_1;
   xy02 = xy02_0 + xy02_1;
   for (; i < N; i++) {
      xy01 = MAC16_16(xy01, x[i], y01[i]);
      xy02 = MAC16_16(xy02, x[i], y02[i]);
   }
   *xy1 = xy01;
   *xy2 = xy02;
}

#endif

opus_val32 celt_inner_prod_neon(const opus_val16 *x, const opus_val16 *y, int N)
{
    int i;
    opus_val32 xy;
    float32x4_t xy_f32x4 = vdupq_n_f32(0);
    float32x2_t xy_f32x2;

    for (i = 0; i < N - 7; i += 8) {
        float32x4_t x_f32x4, y_f32x4;
        x_f32x4  = vld1q_f32(&x[i]);
        y_f32x4  = vld1q_f32(&y[i]);
        xy_f32x4 = vmlaq_f32(xy_f32x4, x_f32x4, y_f32x4);
        x_f32x4  = vld1q_f32(&x[i + 4]);
        y_f32x4  = vld1q_f32(&y[i + 4]);
        xy_f32x4 = vmlaq_f32(xy_f32x4, x_f32x4, y_f32x4);
    }

    if (N - i >= 4) {
        const float32x4_t x_f32x4 = vld1q_f32(&x[i]);
        const float32x4_t y_f32x4 = vld1q_f32(&y[i]);
        xy_f32x4 = vmlaq_f32(xy_f32x4, x_f32x4, y_f32x4);
        i += 4;
    }

    xy_f32x2 = vadd_f32(vget_low_f32(xy_f32x4), vget_high_f32(xy_f32x4));
    xy_f32x2 = vpadd_f32(xy_f32x2, xy_f32x2);
    xy       = vget_lane_f32(xy_f32x2, 0);

    for (; i < N; i++) {
        xy = MAC16_16(xy, x[i], y[i]);
    }

#ifdef OPUS_CHECK_ASM
    celt_assert(ABS32(celt_inner_prod_neon_float_c_simulation(x, y, N) - xy) <= VERY_SMALL);
#endif

    return xy;
}

void dual_inner_prod_neon(const opus_val16 *x, const opus_val16 *y01, const opus_val16 *y02,
        int N, opus_val32 *xy1, opus_val32 *xy2)
{
    int i;
    opus_val32 xy01, xy02;
    float32x4_t xy01_f32x4 = vdupq_n_f32(0);
    float32x4_t xy02_f32x4 = vdupq_n_f32(0);
    float32x2_t xy01_f32x2, xy02_f32x2;

    for (i = 0; i < N - 7; i += 8) {
        float32x4_t x_f32x4, y01_f32x4, y02_f32x4;
        x_f32x4    = vld1q_f32(&x[i]);
        y01_f32x4  = vld1q_f32(&y01[i]);
        y02_f32x4  = vld1q_f32(&y02[i]);
        xy01_f32x4 = vmlaq_f32(xy01_f32x4, x_f32x4, y01_f32x4);
        xy02_f32x4 = vmlaq_f32(xy02_f32x4, x_f32x4, y02_f32x4);
        x_f32x4    = vld1q_f32(&x[i + 4]);
        y01_f32x4  = vld1q_f32(&y01[i + 4]);
        y02_f32x4  = vld1q_f32(&y02[i + 4]);
        xy01_f32x4 = vmlaq_f32(xy01_f32x4, x_f32x4, y01_f32x4);
        xy02_f32x4 = vmlaq_f32(xy02_f32x4, x_f32x4, y02_f32x4);
    }

    if (N - i >= 4) {
        const float32x4_t x_f32x4   = vld1q_f32(&x[i]);
        const float32x4_t y01_f32x4 = vld1q_f32(&y01[i]);
        const float32x4_t y02_f32x4 = vld1q_f32(&y02[i]);
        xy01_f32x4 = vmlaq_f32(xy01_f32x4, x_f32x4, y01_f32x4);
        xy02_f32x4 = vmlaq_f32(xy02_f32x4, x_f32x4, y02_f32x4);
        i += 4;
    }

    xy01_f32x2 = vadd_f32(vget_low_f32(xy01_f32x4), vget_high_f32(xy01_f32x4));
    xy02_f32x2 = vadd_f32(vget_low_f32(xy02_f32x4), vget_high_f32(xy02_f32x4));
    xy01_f32x2 = vpadd_f32(xy01_f32x2, xy01_f32x2);
    xy02_f32x2 = vpadd_f32(xy02_f32x2, xy02_f32x2);
    xy01       = vget_lane_f32(xy01_f32x2, 0);
    xy02       = vget_lane_f32(xy02_f32x2, 0);

    for (; i < N; i++) {
        xy01 = MAC16_16(xy01, x[i], y01[i]);
        xy02 = MAC16_16(xy02, x[i], y02[i]);
    }
    *xy1 = xy01;
    *xy2 = xy02;

#ifdef OPUS_CHECK_ASM
    {
        opus_val32 xy1_c, xy2_c;
        dual_inner_prod_neon_float_c_simulation(x, y01, y02, N, &xy1_c, &xy2_c);
        celt_assert(ABS32(xy1_c - *xy1) <= VERY_SMALL);
        celt_assert(ABS32(xy2_c - *xy2) <= VERY_SMALL);
    }
#endif
}

#endif
