

#include "fwd_short_complex_rot.h"
#include "digit_reversal_tables.h"
#include "imdct_fxp.h"
#include "pv_normalize.h"

Int fwd_short_complex_rot(
    Int32 *Data_in,
    Int32 *Data_out,
    Int32  max)

{
    Int     i;
    Int16     I;
    const   Int16 *pTable;
    const   Int32 *p_rotate;

    Int32   *pData_in_1;
    Int     exp;
    Int32   temp_re;
    Int32   temp_im;

    Int32   cos_n;
    Int32   sin_n;
    Int32   temp_re_32;
    Int32   temp_im_32;

    Int32   *pData_in_ref;

    Int32   *pData_out_1;
    Int32   *pData_out_2;
    Int32   *pData_out_3;
    Int32   *pData_out_4;

    pTable    =  digit_reverse_64;
    p_rotate  =  exp_rotation_N_256;

    pData_in_ref  =  Data_in;

    exp = 16 - pv_normalize(max);

    if (exp < 0)
    {
        exp = 0;
    }

    pData_out_1 = Data_out;
    pData_out_2 = &Data_out[TWICE_FWD_SHORT_CX_ROT_LENGTH_m_1];
    pData_out_3 = &Data_out[TWICE_FWD_SHORT_CX_ROT_LENGTH];
    pData_out_4 = &Data_out[FOUR_FWD_SHORT_CX_ROT_LENGTH_m_1];

    for (i = FWD_SHORT_CX_ROT_LENGTH; i != 0; i--)
    {

        I = *pTable++;
        pData_in_1 = pData_in_ref + I;

        sin_n = *p_rotate++;
        cos_n = sin_n >> 16;
        sin_n = sin_n & 0xFFFF;

        temp_re =  *(pData_in_1++) >> exp;
        temp_im =  *(pData_in_1) >> exp;

        temp_re_32 = (temp_re * cos_n + temp_im * sin_n) >> 16;
        temp_im_32 = (temp_im * cos_n - temp_re * sin_n) >> 16;

        *(pData_out_1++) = - temp_re_32;
        *(pData_out_2--) =   temp_im_32;
        *(pData_out_3++) = - temp_im_32;
        *(pData_out_4--) =   temp_re_32;

        pData_out_1++;
        pData_out_2--;
        pData_out_3++;
        pData_out_4--;

    }

    return (exp);
}
