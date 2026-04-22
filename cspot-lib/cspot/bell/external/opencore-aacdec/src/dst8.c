

#include "config.h"

#ifdef AAC_PLUS

#include "dst8.h"

#include "fxp_mul32.h"

#define Qfmt15(x)   (Int16)(x*((Int32)1<<15) + (x>=0?0.5F:-0.5F))

#define R_SHIFT     29
#define Qfmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

#define Qfmt31(x)   (Int32)(x*0x7FFFFFFF + (x>=0?0.5F:-0.5F))

void dst_8(Int32 vec[])
{

    Int32 temp1;
    Int32 temp2;
    Int32 temp3;
    Int32 temp4;
    Int32 temp5;
    Int32 temp6;
    Int32 temp7;
    Int32 tmp_a;
    Int32 tmp_aa;
    Int32 tmp_b;
    Int32 tmp_bb;
    Int32 tmp_c;
    Int32 tmp_cc;
    Int32 tmp_d;
    Int32 tmp_dd;

    temp1 = fxp_mul32_by_16(vec[1], Qfmt15(0.50979557910416F));
    temp2 = fxp_mul32_by_16(vec[2], Qfmt15(0.54119610014620F));
    temp3 = fxp_mul32_by_16(vec[3], Qfmt15(0.60134488693505F));
    temp5 = fxp_mul32_by_16(vec[5], Qfmt15(0.89997622313642F));
    temp6 = fxp_mul32_by_16(vec[6] << 1, Qfmt15(0.65328148243819F));
    temp7 = vec[7] + fxp_mul32_Q31(vec[7], Qfmt31(0.56291544774152F));

    tmp_a = fxp_mul32_Q31((temp2 + temp6) << 1, Qfmt31(0.70710678118655F));
    tmp_b = (temp2 - temp6) + tmp_a;

    temp4 = fxp_mul32_by_16(vec[4], Qfmt15(0.70710678118655F));
    vec[0] =   tmp_a + temp4;
    vec[1] =   tmp_b + temp4;
    vec[2] =   tmp_b - temp4;
    vec[3] =   tmp_a - temp4;

    tmp_a  = fxp_mul32_by_16((temp1 + temp7) << 1, Qfmt15(0.54119610014620F));
    tmp_aa = (temp1 - temp7);
    tmp_bb = (temp5 - temp3);
    temp5  = fxp_mul32_Q29((temp5 + temp3), Qfmt(1.30656296487638F));

    tmp_c  = fxp_mul32_by_16((tmp_a + temp5) << 1, Qfmt15(0.70710678118655F));
    tmp_cc =  tmp_a - temp5;

    tmp_d  = fxp_mac32_by_16((tmp_aa - tmp_bb) << 1, Qfmt15(0.70710678118655F), tmp_c);
    tmp_dd = (tmp_aa + tmp_bb);

    tmp_dd +=  tmp_c;
    tmp_a   =  tmp_d  + tmp_cc;
    vec[5]  =  tmp_a  - vec[2];
    vec[2] +=  tmp_a;

    temp5   =  tmp_dd + tmp_cc;

    vec[4]  =  temp5  - vec[3];
    vec[3] +=  temp5;
    vec[7]  =  tmp_c  - vec[0];
    vec[0] +=  tmp_c;
    vec[6]  =  tmp_d  - vec[1];
    vec[1] +=  tmp_d;

}

#endif
