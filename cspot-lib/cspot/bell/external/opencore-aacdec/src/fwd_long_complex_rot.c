

#include "fwd_long_complex_rot.h"
#include "digit_reversal_tables.h"
#include "imdct_fxp.h"
#include "pv_normalize.h"

#include "fxp_mul32.h"

Int fwd_long_complex_rot(
    Int32 *Data_in,
    Int32 *Data_out,
    Int32  max)
{
    Int     i;
    const   Int32 *p_rotate;
    Int32   temp_re;
    Int32   temp_im;
    Int32   *pData_in_ref1;
    Int32   *pData_in_ref2;
    Int32   exp_jw;
    Int32   temp_re_32;
    Int32   temp_im_32;

    Int32   *pData_out_1;
    Int32   *pData_out_2;
    Int32   *pData_out_3;
    Int32   *pData_out_4;

    Int32 *pData_in_1;
    Int32 *pData_in_2;

    Int     exp;

    p_rotate       =  exp_rotation_N_2048;

    pData_in_ref1  =  Data_in;
    pData_in_ref2  = &Data_in[TWICE_FWD_LONG_CX_ROT_LENGTH];

    pData_out_1 = Data_out;
    pData_out_2 = &Data_out[LONG_WINDOW_LENGTH_m_1];
    pData_out_3 = &Data_out[LONG_WINDOW_LENGTH];
    pData_out_4 = &Data_out[TWICE_LONG_WINDOW_LENGTH_m_1];

    exp = 16 - pv_normalize(max);

    if (exp < 0)
    {
        exp = 0;
    }

    pData_in_1 = pData_in_ref1;
    pData_in_2 = pData_in_ref2;

    for (i = FWD_LONG_CX_ROT_LENGTH; i != 0; i--)
    {

        exp_jw = *p_rotate++;

        temp_re =  *(pData_in_1++) >> exp;
        temp_im =  *(pData_in_1++) >> exp;

        temp_re_32  = (cmplx_mul32_by_16(temp_re,   temp_im,  exp_jw));
        temp_im_32  = (cmplx_mul32_by_16(temp_im,  -temp_re,  exp_jw));

        *(pData_out_1++) = - temp_re_32;
        *(pData_out_2--) =   temp_im_32;
        *(pData_out_3++) = - temp_im_32;
        *(pData_out_4--) =   temp_re_32;

        pData_out_1++;
        pData_out_2--;
        pData_out_3++;
        pData_out_4--;

        exp_jw = *p_rotate++;

        temp_re =  *(pData_in_2++) >> exp;
        temp_im =  *(pData_in_2++) >> exp;

        temp_re_32  = (cmplx_mul32_by_16(temp_re,   temp_im,  exp_jw));
        temp_im_32  = (cmplx_mul32_by_16(temp_im,  -temp_re,  exp_jw));

        *(pData_out_1++) = - temp_re_32;
        *(pData_out_2--) =   temp_im_32;
        *(pData_out_3++) = - temp_im_32;
        *(pData_out_4--) =   temp_re_32;

        pData_out_1++;
        pData_out_2--;
        pData_out_3++;
        pData_out_4--;

    }

    return (exp + 1);
}
