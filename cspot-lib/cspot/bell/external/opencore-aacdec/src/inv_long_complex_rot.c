

#include "digit_reversal_tables.h"
#include "inv_long_complex_rot.h"
#include "imdct_fxp.h"
#include "inv_long_complex_rot.h"
#include "pv_normalize.h"

#include "fxp_mul32.h"
#include "aac_mem_funcs.h"

Int inv_long_complex_rot(
    Int32 *Data,
    Int32  max)
{
    Int     i;
    Int16     I;
    const   Int32 *p_rotate;
    Int32   temp_re;
    Int32   temp_im;

    Int32    exp_jw;
    Int32   *pData_in_1;
    Int32   *pData_in_2;
    Int     exp;
    Int32   *pData_in_ref1;
    Int32   *pData_in_ref2;

    Int16   temp_re_0;
    Int16   temp_im_0;
    Int16   temp_re_1;
    Int16   temp_im_1;
    Int16   *p_Data_Int_precision;
    Int     n     = 2048;
    Int     n_2   = n >> 1;
    Int     n_4   = n >> 2;
    Int     n_3_4 = n_2 + n_4;

    Int16   *px_1;
    Int16   *px_2;
    Int16   *px_3;
    Int16   *px_4;

    Int16     J;
    const   Int32 *p_rotate2;

    p_rotate    =  &exp_rotation_N_2048[255];
    p_rotate2   =  &exp_rotation_N_2048[256];

    pData_in_ref1  =  Data;
    pData_in_ref2  = &Data[TWICE_INV_LONG_CX_ROT_LENGTH];

    p_Data_Int_precision = (Int16 *)Data;

    exp = 16 - pv_normalize(max);

    I = 255;
    J = 256;

    pData_in_1 = pData_in_ref2 + I;

    px_1 = (Int16 *)pData_in_1;
    px_1++;

    pData_in_2 = pData_in_ref2 + J;

    px_4 = (Int16 *)pData_in_2;

    exp -= 1;

    for (i = INV_LONG_CX_ROT_LENGTH >> 1; i != 0; i--)
    {

        pData_in_2 = pData_in_ref1 + J;

        temp_im =  *(pData_in_2++);
        temp_re =  *(pData_in_2);

        exp_jw = *p_rotate2++;

        temp_re_0  = (Int16)(cmplx_mul32_by_16(temp_re,  -temp_im,  exp_jw) >> exp);
        temp_im_0  = (Int16)(cmplx_mul32_by_16(temp_im,   temp_re,  exp_jw) >> exp);

        pData_in_1 = pData_in_ref2 + I;

        temp_re =  *(pData_in_1--);
        temp_im =  *(pData_in_1);

        exp_jw = *p_rotate--;

        temp_re_1  = (Int16)(cmplx_mul32_by_16(temp_re,  -temp_im,  exp_jw) >> exp);
        temp_im_1  = (Int16)(cmplx_mul32_by_16(temp_im,   temp_re,  exp_jw) >> exp);

        pData_in_2 = pData_in_ref2 + J;
        J += 2;

        temp_im =  *(pData_in_2++);
        temp_re =  *(pData_in_2);

        *(px_1--) =  temp_re_0;
        *(px_1--) =  temp_im_1;
        *(px_4++) =  temp_im_0;
        *(px_4++) =  temp_re_1;

        exp_jw = *p_rotate2++;

        *(px_1--)  = (Int16)(cmplx_mul32_by_16(temp_re,  -temp_im,  exp_jw) >> exp);
        *(px_4++)  = (Int16)(cmplx_mul32_by_16(temp_im,   temp_re,  exp_jw) >> exp);

        pData_in_1 = pData_in_ref1 + I;
        I -= 2;

        temp_re =  *(pData_in_1--);
        temp_im =  *(pData_in_1);

        exp_jw = *p_rotate--;

        *(px_4++)  = (Int16)(cmplx_mul32_by_16(temp_re,  -temp_im,  exp_jw) >> exp);
        *(px_1--)  = (Int16)(cmplx_mul32_by_16(temp_im,   temp_re,  exp_jw) >> exp);

    }

    px_1 = p_Data_Int_precision + n_2 - 1;
    px_2 = p_Data_Int_precision;

    px_4 = p_Data_Int_precision + n_3_4 - 1;

    for (i = 0; i<INV_LONG_CX_ROT_LENGTH >> 1; i++)
    {

        Int16 temp_re_0 = *(px_4--);
        Int16 temp_im_1 = *(px_4--);
        Int16 temp_re_2 = *(px_4--);
        Int16 temp_im_3 = *(px_4--);
        *(px_1--) = temp_re_0;
        *(px_1--) = temp_im_1;
        *(px_1--) = temp_re_2;
        *(px_1--) = temp_im_3;

        *(px_2++) = (-temp_re_0);
        *(px_2++) = (-temp_im_1);
        *(px_2++) = (-temp_re_2);
        *(px_2++) = (-temp_im_3);

    }

    px_4 = p_Data_Int_precision + n_2;

    pv_memcpy(px_4, pData_in_ref2 + 256, TWICE_INV_LONG_CX_ROT_LENGTH*sizeof(*px_4));

    px_3 = p_Data_Int_precision + n - 1;

    for (i = 0; i<INV_LONG_CX_ROT_LENGTH >> 1; i++)
    {

        Int16 temp_im_0 = *(px_4++);
        Int16 temp_re_1 = *(px_4++);
        Int16 temp_im_2 = *(px_4++);
        Int16 temp_re_3 = *(px_4++);
        *(px_3--) =  temp_im_0;
        *(px_3--) =  temp_re_1;
        *(px_3--) =  temp_im_2;
        *(px_3--) =  temp_re_3;

    }

    return (exp + 1);
}

