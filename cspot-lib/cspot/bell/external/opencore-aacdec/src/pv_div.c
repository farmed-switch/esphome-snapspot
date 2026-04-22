

#include    "config.h"

#ifdef AAC_PLUS

#include "pv_audio_type_defs.h"
#include "fxp_mul32.h"
#include "pv_div.h"
#include "pv_normalize.h"

void pv_div(Int32 x, Int32 y, Quotient *result)
{

    Int32 quotient;
    Int32 i;
    Int32 j;
    Int32 y_ov_y_hi;
    Int32 flag = 0;

    result->shift_factor = 0;

    if (y == 0)
    {
        x = 0;
    }

    if (y < 0)
    {
        y = -y;
        flag ^= 1;
    }

    if (x < 0)
    {
        x = -x;
        flag ^= 1;
    }

    if (x != 0)
    {

        i = pv_normalize(x);

        x <<= i;

        j = pv_normalize(y);

        y <<= j;

        result->shift_factor = i - j;

        quotient = (0x40000000 / (y >> 15));

        y_ov_y_hi = fxp_mul32_Q15(y, quotient);

        y_ov_y_hi = 0x7FFFFFFF - y_ov_y_hi;
        y_ov_y_hi = fxp_mul32_Q14(quotient,  y_ov_y_hi);
        i  = fxp_mul32_Q31(y_ov_y_hi,  x) << 1;

        result->quotient = flag ? -i : i;
    }
    else
    {
        result->quotient = 0;
    }

}

#endif

