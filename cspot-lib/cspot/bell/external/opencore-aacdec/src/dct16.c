

#include "config.h"

#ifdef AAC_PLUS

#include "dct16.h"
#include "fxp_mul32.h"

#define Qfmt_31(a)   (Int32)(a*0x7FFFFFFF + (a>=0?0.5F:-0.5F))

#define Qfmt15(x)   (Int16)(x*((Int32)1<<15) + (x>=0?0.5F:-0.5F))

void dct_16(Int32 vec[], Int flag)
{
    Int32 tmp0;
    Int32 tmp1;
    Int32 tmp2;
    Int32 tmp3;
    Int32 tmp4;
    Int32 tmp5;
    Int32 tmp6;
    Int32 tmp7;
    Int32 tmp_o0;
    Int32 tmp_o1;
    Int32 tmp_o2;
    Int32 tmp_o3;
    Int32 tmp_o4;
    Int32 tmp_o5;
    Int32 tmp_o6;
    Int32 tmp_o7;
    Int32 itmp_e0;
    Int32 itmp_e1;
    Int32 itmp_e2;

    tmp_o0 = fxp_mul32_by_16((vec[ 0] - vec[15]), Qfmt15(0.50241928618816F));
    tmp0   =  vec[ 0] + vec[15];

    tmp_o7 = fxp_mul32_Q31((vec[ 7] - vec[ 8]) << 3, Qfmt_31(0.63764357733614F));
    tmp7   =  vec[ 7] + vec[ 8];

    itmp_e0 = (tmp0 + tmp7);
    tmp7    = fxp_mul32_by_16((tmp0 - tmp7), Qfmt15(0.50979557910416F));

    tmp_o1 = fxp_mul32_by_16((vec[ 1] - vec[14]), Qfmt15(0.52249861493969F));
    tmp1   =  vec[ 1] + vec[14];
    tmp_o6 = fxp_mul32_by_16((vec[ 6] - vec[ 9]) << 1, Qfmt15(0.86122354911916F));
    tmp6   =  vec[ 6] + vec[ 9];

    itmp_e1 = (tmp1 + tmp6);
    tmp6    = fxp_mul32_by_16((tmp1 - tmp6), Qfmt15(0.60134488693505F));

    tmp_o2 = fxp_mul32_by_16((vec[ 2] - vec[13]), Qfmt15(0.56694403481636F));
    tmp2   =  vec[ 2] + vec[13];
    tmp_o5 = fxp_mul32_by_16((vec[ 5] - vec[10]) << 1, Qfmt15(0.53033884299517F));
    tmp5   =  vec[ 5] + vec[10];

    itmp_e2 = (tmp2 + tmp5);
    tmp5    = fxp_mul32_by_16((tmp2 - tmp5), Qfmt15(0.89997622313642F));

    tmp_o3 = fxp_mul32_by_16((vec[ 3] - vec[12]), Qfmt15(0.64682178335999F));
    tmp3   =  vec[ 3] + vec[12];
    tmp_o4 = fxp_mul32_by_16((vec[ 4] - vec[11]), Qfmt15(0.78815462345125F));
    tmp4   =  vec[ 4] + vec[11];

    tmp1   = (tmp3 + tmp4);
    tmp4   =  fxp_mul32_Q31((tmp3 - tmp4) << 2, Qfmt_31(0.64072886193538F));

    tmp0 = (itmp_e0 + tmp1);
    tmp1 = fxp_mul32_by_16((itmp_e0 - tmp1), Qfmt15(0.54119610014620F));

    tmp3 = fxp_mul32_by_16((itmp_e1 - itmp_e2) << 1, Qfmt15(0.65328148243819F));
    tmp2 = (itmp_e1 + itmp_e2);

    vec[ 0]  = (tmp0 + tmp2) >> 1;
    vec[ 8]  = fxp_mul32_by_16((tmp0 - tmp2), Qfmt15(0.70710678118655F));
    vec[12]  = fxp_mul32_by_16((tmp1 - tmp3) << 1, Qfmt15(0.70710678118655F));
    vec[ 4]  =  tmp1 + tmp3;
    vec[ 4] +=  vec[12];

    tmp1 = fxp_mul32_by_16((tmp7 - tmp4) << 1, Qfmt15(0.54119610014620F));
    tmp7 += tmp4;
    tmp3 = fxp_mul32_Q31((tmp6 - tmp5) << 2, Qfmt_31(0.65328148243819F));

    tmp6 += tmp5;

    vec[10]  = fxp_mul32_by_16((tmp7 - tmp6) << 1, Qfmt15(0.70710678118655F));
    vec[ 2]  =  tmp7 + tmp6;
    vec[14]  = fxp_mul32_by_16((tmp1 - tmp3) << 1, Qfmt15(0.70710678118655F));

    tmp1    +=  tmp3 + vec[14];
    vec[ 2] +=  tmp1;
    vec[ 6]  =  tmp1 + vec[10];

    vec[10] += vec[14];

    tmp7 = tmp_o0 + tmp_o7;
    tmp_o7 = fxp_mul32_by_16((tmp_o0 - tmp_o7) << 1, Qfmt15(0.50979557910416F));

    tmp6 = tmp_o1 + tmp_o6;
    tmp_o1 = fxp_mul32_by_16((tmp_o1 - tmp_o6) << 1, Qfmt15(0.60134488693505F));

    tmp5 = tmp_o2 + tmp_o5;
    tmp_o5 = fxp_mul32_by_16((tmp_o2 - tmp_o5) << 1, Qfmt15(0.89997622313642F));

    tmp4 = tmp_o3 + tmp_o4;

    tmp_o3 = fxp_mul32_Q31((tmp_o3 - tmp_o4) << 3, Qfmt_31(0.6407288619354F));

    if (!flag)
    {
        tmp7   = -tmp7;
        tmp_o7 = -tmp_o7;
        tmp6   = -tmp6;
        tmp_o1 = -tmp_o1;
        tmp5   = -tmp5;
        tmp_o5 = -tmp_o5;
        tmp4   = -tmp4;
        tmp_o3 = -tmp_o3;
    }

    tmp1 = fxp_mul32_by_16((tmp7 - tmp4) << 1, Qfmt15(0.54119610014620F));
    tmp0 =  tmp7 + tmp4;
    tmp3 = fxp_mul32_Q31((tmp6 - tmp5) << 2, Qfmt_31(0.65328148243819F));
    tmp2 =  tmp6 + tmp5;

    vec[ 9]  = fxp_mul32_Q31((tmp0 - tmp2) << 1, Qfmt_31(0.70710678118655F));
    vec[ 1]  =  tmp0 + tmp2;
    vec[13]  = fxp_mul32_Q31((tmp1 - tmp3) << 1, Qfmt_31(0.70710678118655F));

    vec[ 5]  =  tmp1 + tmp3 + vec[13];

    tmp0 =  tmp_o7 + tmp_o3;
    tmp1 = fxp_mul32_by_16((tmp_o7 - tmp_o3) << 1, Qfmt15(0.54119610014620F));
    tmp2 =  tmp_o1 + tmp_o5;
    tmp3 = fxp_mul32_Q31((tmp_o1 - tmp_o5) << 2, Qfmt_31(0.65328148243819F));

    vec[11]  = fxp_mul32_Q31((tmp0 - tmp2) << 1, Qfmt_31(0.70710678118655F));
    vec[ 3]  =  tmp0 + tmp2;
    vec[15]  = fxp_mul32_Q31((tmp1 - tmp3) << 1, Qfmt_31(0.70710678118655F));
    vec[ 7]  =  tmp1 + tmp3 + vec[15];

    vec[ 3] += vec[ 7];
    vec[ 7] += vec[11];
    vec[11] += vec[15];

    vec[ 1] += vec[ 3];
    vec[ 3] += vec[ 5];
    vec[ 5] += vec[ 7];
    vec[ 7] += vec[ 9];
    vec[ 9] += vec[11];
    vec[11] += vec[13];
    vec[13] += vec[15];

}

#endif

