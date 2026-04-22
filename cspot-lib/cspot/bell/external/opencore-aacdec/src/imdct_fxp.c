

#include "pv_audio_type_defs.h"
#include "imdct_fxp.h"

#include "mix_radix_fft.h"
#include "digit_reversal_tables.h"
#include "fft_rx4.h"
#include "inv_short_complex_rot.h"
#include "inv_long_complex_rot.h"
#include "pv_normalize.h"
#include "fxp_mul32.h"
#include "aac_mem_funcs.h"

#include "window_block_fxp.h"

#define ERROR_IN_FRAME_SIZE 10

Int imdct_fxp(Int32   data_quant[],
              Int32   freq_2_time_buffer[],
              const   Int     n,
              Int     Q_format,
              Int32   max)
{

    Int32     exp_jw;
    Int     shift = 0;

    const   Int32 *p_rotate;
    const   Int32 *p_rotate_2;

    Int32   *p_data_1;
    Int32   *p_data_2;

    Int32   temp_re32;
    Int32   temp_im32;

    Int     shift1 = 0;
    Int32   temp1;
    Int32   temp2;

    Int     k;
    Int     n_2   = n >> 1;
    Int     n_4   = n >> 2;

    if (max != 0)
    {

        switch (n)
        {
            case SHORT_WINDOW_TYPE:
                p_rotate = exp_rotation_N_256;
                shift = 21;
                break;

            case LONG_WINDOW_TYPE:
                p_rotate = exp_rotation_N_2048;
                shift = 24;
                break;

            default:

                return(ERROR_IN_FRAME_SIZE);

        }

        p_data_1 =  data_quant;
        p_data_2 = &data_quant[n_2 - 1];

        p_rotate_2 = &p_rotate[n_4-1];

        shift1 = pv_normalize(max) - 1;
        Q_format -= (16 - shift1);
        max = 0;

        if (shift1 >= 0)
        {
            temp_re32 =   *(p_data_1++) << shift1;
            temp_im32 =   *(p_data_2--) << shift1;

            for (k = n_4 >> 1; k != 0; k--)
            {

                exp_jw = *p_rotate++;

                temp1      =  cmplx_mul32_by_16(temp_im32, -temp_re32, exp_jw);
                temp2      = -cmplx_mul32_by_16(temp_re32,  temp_im32, exp_jw);

                temp_im32 =   *(p_data_1--) << shift1;
                temp_re32 =   *(p_data_2--) << shift1;
                *(p_data_1++) = temp1;
                *(p_data_1++) = temp2;
                max         |= (temp1 >> 31) ^ temp1;
                max         |= (temp2 >> 31) ^ temp2;

                exp_jw = *p_rotate_2--;

                temp1      =  cmplx_mul32_by_16(temp_im32, -temp_re32, exp_jw);
                temp2      = -cmplx_mul32_by_16(temp_re32,  temp_im32, exp_jw);

                temp_re32 =   *(p_data_1++) << shift1;
                temp_im32 =   *(p_data_2--) << shift1;

                *(p_data_2 + 2) = temp1;
                *(p_data_2 + 3) = temp2;
                max         |= (temp1 >> 31) ^ temp1;
                max         |= (temp2 >> 31) ^ temp2;

            }
        }
        else
        {
            temp_re32 =   *(p_data_1++) >> 1;
            temp_im32 =   *(p_data_2--) >> 1;

            for (k = n_4 >> 1; k != 0; k--)
            {

                exp_jw = *p_rotate++;

                temp1      =  cmplx_mul32_by_16(temp_im32, -temp_re32, exp_jw);
                temp2      = -cmplx_mul32_by_16(temp_re32,  temp_im32, exp_jw);

                temp_im32 =   *(p_data_1--) >> 1;
                temp_re32 =   *(p_data_2--) >> 1;
                *(p_data_1++) = temp1;
                *(p_data_1++) = temp2;

                max         |= (temp1 >> 31) ^ temp1;
                max         |= (temp2 >> 31) ^ temp2;

                exp_jw = *p_rotate_2--;

                temp1      =  cmplx_mul32_by_16(temp_im32, -temp_re32, exp_jw);
                temp2      = -cmplx_mul32_by_16(temp_re32,  temp_im32, exp_jw);

                temp_re32 =   *(p_data_1++) >> 1;
                temp_im32 =   *(p_data_2--) >> 1;

                *(p_data_2 + 3) = temp2;
                *(p_data_2 + 2) = temp1;

                max         |= (temp1 >> 31) ^ temp1;
                max         |= (temp2 >> 31) ^ temp2;

            }
        }

        if (n != SHORT_WINDOW_TYPE)
        {

            shift -= mix_radix_fft(data_quant,
                                   &max);

            shift -= inv_long_complex_rot(data_quant,
                                          max);

        }
        else
        {

            shift -= fft_rx4_short(data_quant,   &max);

            shift -= inv_short_complex_rot(data_quant,
                                           freq_2_time_buffer,
                                           max);

            pv_memcpy(data_quant,
                      freq_2_time_buffer,
                      SHORT_WINDOW*sizeof(*data_quant));
        }

    }
    else
    {
        Q_format = ALL_ZEROS_BUFFER;
    }

    return(shift + Q_format);

}
