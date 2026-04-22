

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include "pv_audio_type_defs.h"
#include "ps_fft_rx8.h"

#include    "fxp_mul32.h"

#define R_SHIFT     29
#define Q29_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

void ps_fft_rx8(Int32 Re[], Int32 Im[], Int32 scratch_mem[])

{

    Int     i;
    Int32   *Q = &scratch_mem[0];
    Int32   *Z = &scratch_mem[16];
    Int32   temp1;
    Int32   temp2;
    Int32   temp3;
    Int32   temp4;
    Int32   aux_r[2];
    Int32   aux_i[2];
    Int32   *pt_r1 = &Re[0];
    Int32   *pt_r2 = &Re[4];
    Int32   *pt_i1 = &Im[0];
    Int32   *pt_i2 = &Im[4];

    Int32   *pt_Q = Q;
    Int32   *pt_Z = Z;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_i1++);
    temp4 = *(pt_i2++);

    *(pt_Q++) = temp1 + temp2;
    *(pt_Q++) = temp3 + temp4;
    *(pt_Q++) = temp1 - temp2;
    *(pt_Q++) = temp3 - temp4;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_i1++);
    temp4 = *(pt_i2++);

    *(pt_Q++) = temp1 + temp2;
    *(pt_Q++) = temp3 + temp4;
    aux_r[0]  = temp1 - temp2;
    aux_i[0]  = temp3 - temp4;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_i1++);
    temp4 = *(pt_i2++);

    *(pt_Q++) = temp1 + temp2;
    *(pt_Q++) = temp3 + temp4;
    *(pt_Q++) = temp4 - temp3;
    *(pt_Q++) = temp1 - temp2;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_i1++);
    temp4 = *(pt_i2++);

    *(pt_Q++) = temp1 + temp2;
    *(pt_Q++) = temp3 + temp4;
    aux_r[1]  = temp1 - temp2;
    aux_i[1]  = temp3 - temp4;

    *(pt_Q++) = fxp_mul32_Q29((aux_r[0] - aux_r[1]), Q29_fmt(0.70710678118655f));
    *(pt_Q++) = fxp_mul32_Q29((aux_i[0] - aux_i[1]), Q29_fmt(0.70710678118655f));

    *(pt_Q++) =  fxp_mul32_Q29((aux_i[0] + aux_i[1]), Q29_fmt(-0.70710678118655f));
    *(pt_Q) =  fxp_mul32_Q29((aux_r[0] + aux_r[1]), Q29_fmt(0.70710678118655f));

    pt_r1 = &Q[0];
    pt_r2 = &Q[6];

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_r1++);
    temp4 = *(pt_r2++);

    *(pt_Z++) = temp1 + temp2;
    *(pt_Z++) = temp3 + temp4;
    aux_r[0]  = temp1 - temp2;
    aux_i[0]  = temp3 - temp4;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_r1++);
    temp4 = *(pt_r2++);

    *(pt_Z++) = temp1 + temp2;
    *(pt_Z++) = temp3 + temp4;
    *(pt_Z++) = aux_r[0];
    *(pt_Z++) = aux_i[0];
    *(pt_Z++) = temp1 - temp2;
    *(pt_Z++) = temp3 - temp4;

    temp1 = *(pt_r1++);
    temp2 = *(pt_r2++);
    temp3 = *(pt_r1);
    temp4 = *(pt_r2++);

    *(pt_Z++) = temp1 + temp2;
    *(pt_Z++) = temp3 + temp4;
    aux_r[0]  = temp1 - temp2;
    aux_i[0]  = temp3 - temp4;

    temp1 = *(pt_r2++);
    temp3 = *(pt_r2++);
    temp2 = *(pt_r2++);
    temp4 = *(pt_r2);

    *(pt_Z++) = temp1 + temp2;
    *(pt_Z++) = temp3 + temp4;

    *(pt_Z++) = -aux_i[0];
    *(pt_Z++) =  aux_r[0];

    *(pt_Z++) =  temp2 - temp1;
    *(pt_Z) =  temp4 - temp3;

    pt_Z = &Z[0];
    pt_Q = &Z[8];

    pt_r1 = &Re[0];
    pt_r2 = &Re[4];
    pt_i1 = &Im[0];
    pt_i2 = &Im[4];

    for (i = 4; i != 0; i--)
    {
        temp1 = *(pt_Z++);
        temp2 = *(pt_Q++);
        temp3 = *(pt_Z++);
        temp4 = *(pt_Q++);

        *(pt_r1++) = temp1 + temp2;
        *(pt_i1++) = temp3 + temp4;
        *(pt_r2++) = temp1 - temp2;
        *(pt_i2++) = temp3 - temp4;
    }

}

#endif

#endif
