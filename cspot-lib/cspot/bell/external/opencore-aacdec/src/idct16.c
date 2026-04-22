

#include    "config.h"

#ifdef AAC_PLUS

#include "idct16.h"
#include "idct8.h"

#include "fxp_mul32.h"

#define R_SHIFT     28
#define Qfmt(x)     (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))
#define Qfmt31(x)   (Int32)(x*(0x7FFFFFFF) + (x>=0?0.5F:-0.5F))

const Int32 CosTable_8i[8] =
{
    Qfmt31(0.50241928618816F),   Qfmt31(0.52249861493969F),
    Qfmt31(0.56694403481636F),   Qfmt31(0.64682178335999F),
    Qfmt(0.78815462345125F),   Qfmt(1.06067768599035F),
    Qfmt(1.72244709823833F),   Qfmt(5.10114861868916F)
};

void idct_16(Int32 vec[], Int32 scratch_mem[])
{
    Int32 *temp_even = scratch_mem;

    Int32 i;
    const Int32 *pt_cos = CosTable_8i;
    Int32 tmp1, tmp2;
    Int32 *pt_even = temp_even;
    Int32 *pt_odd  = vec;
    Int32 *pt_vec  = vec;

    Int32 tmp3;
    Int32 *pt_vecN_1;

    *(pt_even++) = *(pt_vec++);
    tmp1         = *(pt_vec++);
    *(pt_odd++) = tmp1;

    for (i = 2; i != 0; i--)
    {
        *(pt_even++) = *(pt_vec++);
        tmp2         = *(pt_vec++);
        *(pt_even++) = *(pt_vec++);
        tmp3         = *(pt_vec++);
        *(pt_odd++) = tmp2 + tmp1;
        *(pt_odd++) = tmp3 + tmp2;
        tmp1         = tmp3;
    }

    *(pt_even++) = *(pt_vec++);
    tmp2         = *(pt_vec++);
    *(pt_even++) = *(pt_vec++);
    tmp3         = *(pt_vec++);
    *(pt_odd++) = tmp2 + tmp1;
    *(pt_odd++) = tmp3 + tmp2;

    *(pt_even)   = *(pt_vec++);
    *(pt_odd++) = *(pt_vec) + tmp3;

    idct_8(temp_even);
    idct_8(vec);

    pt_cos = &CosTable_8i[7];

    pt_vec  = &vec[7];

    pt_even = &temp_even[7];
    pt_vecN_1  = &vec[8];

    tmp1 = *(pt_even--);

    for (i = 2; i != 0; i--)
    {
        tmp3  = fxp_mul32_Q28(*(pt_vec), *(pt_cos--));
        tmp2 = *(pt_even--);
        *(pt_vecN_1++)  = tmp1 - tmp3;
        *(pt_vec--)     = tmp1 + tmp3;
        tmp3  = fxp_mul32_Q28(*(pt_vec), *(pt_cos--));
        tmp1 = *(pt_even--);
        *(pt_vecN_1++)  = tmp2 - tmp3;
        *(pt_vec--)     = tmp2 + tmp3;
    }

    tmp3  = fxp_mul32_Q31(*(pt_vec), *(pt_cos--)) << 1;
    tmp2 = *(pt_even--);
    *(pt_vecN_1++)  = tmp1 - tmp3;
    *(pt_vec--)     = tmp1 + tmp3;
    tmp3  = fxp_mul32_Q31(*(pt_vec), *(pt_cos--)) << 1;
    tmp1 = *(pt_even--);
    *(pt_vecN_1++)  = tmp2 - tmp3;
    *(pt_vec--)     = tmp2 + tmp3;
    tmp3  = fxp_mul32_Q31(*(pt_vec), *(pt_cos--)) << 1;
    tmp2 = *(pt_even--);
    *(pt_vecN_1++)  = tmp1 - tmp3;
    *(pt_vec--)     = tmp1 + tmp3;
    tmp3  = fxp_mul32_Q31(*(pt_vec), *(pt_cos)) << 1;
    *(pt_vecN_1)  = tmp2 - tmp3;
    *(pt_vec)     = tmp2 + tmp3;

}

#endif
