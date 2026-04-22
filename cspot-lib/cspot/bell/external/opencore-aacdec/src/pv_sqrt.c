

#include    "config.h"

#ifdef AAC_PLUS

#include "pv_audio_type_defs.h"

#include "fxp_mul32.h"
#include "pv_sqrt.h"

#define R_SHIFT     28
#define Q_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 sqrt_table[9] =
{
    Q_fmt(-0.13829740941110F),  Q_fmt(0.95383399963991F),
    Q_fmt(-2.92784603873353F),  Q_fmt(5.27429191920042F),
    Q_fmt(-6.20272445821478F),  Q_fmt(5.04717433019620F),
    Q_fmt(-3.03362807640415F),  Q_fmt(1.86178814410910F),
    Q_fmt(0.16540758699193F)
};

void pv_sqrt(Int32 man, Int32 exp, Root_sq *result, Int32 *sqrt_cache)
{

    Int32   y;
    Int32   xx;
    Int32   nn;
    Int32   i;
    const Int32 *pt_table = sqrt_table;

    if (sqrt_cache[0] == man && sqrt_cache[1] == exp)
    {
        result->root         =        sqrt_cache[2];
        result->shift_factor = (Int16)sqrt_cache[3];
    }
    else
    {

        sqrt_cache[0] = man;
        sqrt_cache[1] = exp;

        if (man > 0)
        {
            xx =  man;
            if (man >= Q_fmt(1.0f))
            {
                nn = exp + 1;
                while ((xx >>= 1) > Q_fmt(1.0f))
                {
                    nn++;
                }
            }
            else if (man < Q_fmt(0.5f))
            {
                nn = exp - 1;
                while ((xx <<= 1) < Q_fmt(0.5f))
                {
                    nn--;
                }
            }
            else
            {
                nn = exp;
            }

            y  = fxp_mul32_Q28(*(pt_table++), xx);

            for (i = 3; i != 0; i--)
            {
                y += *(pt_table++);
                y  = fxp_mul32_Q28(y, xx);
                y += *(pt_table++);
                y  = fxp_mul32_Q28(y, xx);
            }
            y += *(pt_table++);
            y  = fxp_mul32_Q28(y, xx) + *(pt_table++);

            if (nn >= 0)
            {
                if (nn&1)
                {
                    y = fxp_mul32_Q29(y, Q_fmt(1.41421356237310F));
                    result->shift_factor = (nn >> 1) - 28;
                }
                else
                {
                    result->shift_factor = (nn >> 1) - 29;
                }
            }
            else
            {
                if (nn&1)
                {
                    y = fxp_mul32_Q28(y, Q_fmt(0.70710678118655F));
                }

                result->shift_factor = -((-nn) >> 1) - 29;
            }

            result->root = y;

        }
        else
        {
            result->root = 0;
            result->shift_factor = 0;
        }

    }

    sqrt_cache[2] = result->root;
    sqrt_cache[3] = result->shift_factor;

}

#endif

