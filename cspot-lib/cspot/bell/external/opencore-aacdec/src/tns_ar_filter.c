

#include "pv_audio_type_defs.h"
#include "e_tns_const.h"
#include "tns_ar_filter.h"
#include "fxp_mul32.h"

#define MASK_LOW16               0xFFFF
#define UPPER16                      16

Int tns_ar_filter(
    Int32 spec[],
    const Int spec_length,
    const Int direction,
    const Int32 lpc[],
    const Int Q_lpc,
    const Int order)
{

    Int i;
    Int j;

    Int32 temp;

    Int32 y0;

    Int32 *p_state = NULL;

    const Int32 *p_lpc;

    Int shift_up;
    Int shift_down_amount;

    Int32 *p_spec = spec;

    i = 0;
    j = order;

    while (j < 0x010)
    {
        j <<= 1;
        i++;
    }

    shift_down_amount = 4 - i;

    shift_up = UPPER16 - Q_lpc;

    shift_down_amount += shift_up;

    if (direction == -1)
    {
        p_spec += spec_length - 1;

        for (i = order; i != 0; i--)
        {

            y0 = *p_spec >> shift_down_amount;

            p_lpc = lpc;

            for (j = order; j > i; j--)
            {
                temp = *p_state++;
                y0 -= fxp_mul32_Q31(temp, *(p_lpc++)) << shift_up;
            }

            p_state     = p_spec;
            *(p_spec--) = y0;

        }

        if (spec_length > order)
        {
            for (i = (spec_length - order); i != 0; i--)
            {
                y0 = *p_spec >> shift_down_amount;

                p_lpc = &(lpc[0]);

                for (j = order; j != 0; j--)
                {
                    temp = *p_state++;
                    y0 -= fxp_mul32_Q31(temp, *(p_lpc++)) << shift_up;
                }

                p_state     = p_spec;
                *(p_spec--) = y0;

            }
        }

    }
    else
    {
        for (i = order; i != 0; i--)
        {

            p_lpc =  lpc;

            y0 = 0;

            for (j = order; j > i; j--)
            {
                y0 -= fxp_mul32_Q31(*p_state--, *(p_lpc++));
            }

            p_state     = p_spec;

            *(p_spec) = (*p_spec >> shift_down_amount) + (y0 << shift_up);
            p_spec++;
        }

        if (spec_length > order)
        {
            for (i = (spec_length - order); i != 0; i--)
            {
                p_lpc =  lpc;

                y0 = 0;

                for (j = order; j != 0; j--)
                {
                    y0 -= fxp_mul32_Q31(*p_state--, *(p_lpc++));
                }

                p_state     = p_spec;

                *(p_spec) = (*p_spec >> shift_down_amount) + (y0 << shift_up);
                p_spec++;

            }
        }
    }

    return(shift_down_amount);

}
