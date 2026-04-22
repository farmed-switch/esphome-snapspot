

#include "pv_audio_type_defs.h"
#include "mdct_fxp.h"
#include "fft_rx4.h"
#include "mix_radix_fft.h"
#include "fwd_long_complex_rot.h"
#include "fwd_short_complex_rot.h"

#define ERROR_IN_FRAME_SIZE 10

Int mdct_fxp(
    Int32   data_quant[],
    Int32   Q_FFTarray[],
    Int     n)
{

    Int32   temp_re;
    Int32   temp_im;

    Int32   temp_re_32;
    Int32   temp_im_32;

    Int16     cos_n;
    Int16     sin_n;
    Int32     exp_jw;
    Int     shift;

    const Int32 *p_rotate;

    Int32   *p_data_1;
    Int32   *p_data_2;
    Int32   *p_data_3;
    Int32   *p_data_4;

    Int32 *p_Q_FFTarray;

    Int32   max1;

    Int k;
    Int n_2   = n >> 1;
    Int n_4   = n >> 2;
    Int n_8   = n >> 3;
    Int n_3_4 = 3 * n_4;

    switch (n)
    {
        case SHORT_WINDOW_TYPE:
            p_rotate = (Int32 *)exp_rotation_N_256;
            break;

        case LONG_WINDOW_TYPE:
            p_rotate = (Int32 *)exp_rotation_N_2048;
            break;

        default:

            return(ERROR_IN_FRAME_SIZE);

    }

    p_data_1 = &data_quant[n_3_4];
    p_data_2 = &data_quant[n_3_4 - 1];
    p_data_3 = &data_quant[n_4];
    p_data_4 = &data_quant[n_4 - 1];

    p_Q_FFTarray = Q_FFTarray;

    max1 = 0;

    for (k = n_8; k > 0; k--)
    {

        temp_re = (*(p_data_1++) + *(p_data_2--)) >> 1;
        temp_im = (*(p_data_3++) - *(p_data_4--)) >> 1;

        exp_jw = *p_rotate++;

        cos_n = (Int16)(exp_jw >> 16);
        sin_n = (Int16)(exp_jw & 0xFFFF);

        temp_re_32 = temp_re * cos_n + temp_im * sin_n;
        temp_im_32 = temp_im * cos_n - temp_re * sin_n;
        *(p_Q_FFTarray++) = temp_re_32;
        *(p_Q_FFTarray++) = temp_im_32;
        max1         |= (temp_re_32 >> 31) ^ temp_re_32;
        max1         |= (temp_im_32 >> 31) ^ temp_im_32;

        p_data_1++;
        p_data_2--;
        p_data_4--;
        p_data_3++;
    }

    p_data_1 = &data_quant[n - 1];
    p_data_2 = &data_quant[n_2 - 1];
    p_data_3 = &data_quant[n_2];
    p_data_4 =  data_quant;

    for (k = n_8; k > 0; k--)
    {

        temp_re = (*(p_data_2--) - *(p_data_4++)) >> 1;
        temp_im = (*(p_data_1--) + *(p_data_3++)) >> 1;

        p_data_2--;
        p_data_1--;
        p_data_4++;
        p_data_3++;

        exp_jw = *p_rotate++;

        cos_n = (Int16)(exp_jw >> 16);
        sin_n = (Int16)(exp_jw & 0xFFFF);

        temp_re_32 = temp_re * cos_n + temp_im * sin_n;
        temp_im_32 = temp_im * cos_n - temp_re * sin_n;

        *(p_Q_FFTarray++) = temp_re_32;
        *(p_Q_FFTarray++) = temp_im_32;
        max1         |= (temp_re_32 >> 31) ^ temp_re_32;
        max1         |= (temp_im_32 >> 31) ^ temp_im_32;

    }

    p_Q_FFTarray = Q_FFTarray;

    if (max1)
    {

        if (n != SHORT_WINDOW_TYPE)
        {

            shift = mix_radix_fft(
                        Q_FFTarray,
                        &max1);

            shift += fwd_long_complex_rot(
                         Q_FFTarray,
                         data_quant,
                         max1);

        }
        else
        {

            shift = fft_rx4_short(
                        Q_FFTarray,
                        &max1);

            shift += fwd_short_complex_rot(
                         Q_FFTarray,
                         data_quant,
                         max1);
        }

    }
    else
    {
        shift = -31;
    }

    return (12 - shift);

}

