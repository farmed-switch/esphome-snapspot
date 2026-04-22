

#include "digit_reversal_tables.h"
#include "imdct_fxp.h"
#include "inv_short_complex_rot.h"
#include "pv_normalize.h"
#include "fxp_mul32.h"

Int inv_short_complex_rot(
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

    Int32   exp_jw;
    Int16   *pData_re;
    Int16   *pData_im;
    Int32   *pData_in_ref;

    Int16   temp_re_0;
    Int16   temp_im_0;
    Int16   temp_re_1;
    Int16   temp_im_1;
    Int16   *p_data_1;
    Int16   *p_data_2;
    Int16   *p_Data_Int_precision;
    Int16   *p_Data_Int_precision_1;
    Int16   *p_Data_Int_precision_2;

    Int     n     = 256;
    Int     n_2   = n >> 1;
    Int     n_4   = n >> 2;
    Int     n_8   = n >> 3;
    Int     n_3_4 = n_2 + n_4;

    p_data_1 = (Int16 *)Data_out;
    p_data_1 += n;
    pData_re  = p_data_1;
    pData_im  = p_data_1 + n_4;

    p_rotate  =  exp_rotation_N_256;
    pTable    =  digit_reverse_64;

    pData_in_ref  =  Data_in;

    exp = 16 - pv_normalize(max);

    if (exp < 0)
    {
        exp = 0;
    }

    exp -= 1;

    for (i = INV_SHORT_CX_ROT_LENGTH; i != 0; i--)
    {

        I = *pTable++;
        pData_in_1 = pData_in_ref + I;

        temp_im =  *(pData_in_1++);
        temp_re =  *(pData_in_1);

        exp_jw = *p_rotate++;

        *(pData_re++)  = (Int16)(cmplx_mul32_by_16(temp_re, -temp_im, exp_jw) >> exp);
        *(pData_im++)  = (Int16)(cmplx_mul32_by_16(temp_im,  temp_re, exp_jw) >> exp);
    }

    p_data_2 = pData_im -  1;

    p_Data_Int_precision = (Int16 *)Data_out;
    p_Data_Int_precision_1 = p_Data_Int_precision + n_3_4 - 1;
    p_Data_Int_precision_2 = p_Data_Int_precision + n_3_4;

    for (i = n_8 >> 1; i != 0; i--)
    {
        temp_re_0 = (*(p_data_1++));
        temp_re_1 = (*(p_data_1++));
        temp_im_0 = (*(p_data_2--));
        temp_im_1 = (*(p_data_2--));

        *(p_Data_Int_precision_1--) =  temp_re_0;
        *(p_Data_Int_precision_1--) =  temp_im_0;
        *(p_Data_Int_precision_1--) =  temp_re_1;
        *(p_Data_Int_precision_1--) =  temp_im_1;

        *(p_Data_Int_precision_2++) =  temp_re_0;
        *(p_Data_Int_precision_2++) =  temp_im_0;
        *(p_Data_Int_precision_2++) =  temp_re_1;
        *(p_Data_Int_precision_2++) =  temp_im_1;

    }

    p_Data_Int_precision_2 = p_Data_Int_precision;

    for (i = n_8 >> 1; i != 0; i--)
    {

        temp_re_0 = (*(p_data_1++));
        temp_re_1 = (*(p_data_1++));
        temp_im_0 = (*(p_data_2--));
        temp_im_1 = (*(p_data_2--));

        *(p_Data_Int_precision_1--) =   temp_re_0;
        *(p_Data_Int_precision_1--) =   temp_im_0;
        *(p_Data_Int_precision_1--) =   temp_re_1;
        *(p_Data_Int_precision_1--) =   temp_im_1;

        *(p_Data_Int_precision_2++) = (Int16)(-temp_re_0);
        *(p_Data_Int_precision_2++) = (Int16)(-temp_im_0);
        *(p_Data_Int_precision_2++) = (Int16)(-temp_re_1);
        *(p_Data_Int_precision_2++) = (Int16)(-temp_im_1);

    }

    return (exp + 1);
}
