

#include    "config.h"

#ifdef AAC_PLUS

#include "pv_log2.h"
#include "fxp_mul32.h"

#define R_SHIFT     20
#define Q_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 log_table[9] =
{
    Q_fmt(-0.00879832091331F),  Q_fmt(0.12022974263833F),
    Q_fmt(-0.72883958314294F),  Q_fmt(2.57909824242332F),
    Q_fmt(-5.90041216630330F),  Q_fmt(9.15023342527264F),
    Q_fmt(-9.90616619500413F),  Q_fmt(8.11228968755409F),
    Q_fmt(-3.41763474309898F)
};

Int32 pv_log2(Int32 z)
{
    const Int32 *pt_table = log_table;
    Int32 y;
    Int32 i;

    Int32 int_log2 = 0;

    if (z > Q_fmt(2.0f))
    {
        while (z > Q_fmt(2.0f))
        {
            z >>= 1;
            int_log2++;
        }
    }
    else if (z < Q_fmt(1.0f))
    {
        {
            while (z < Q_fmt(1.0f))
            {
                z <<= 1;
                int_log2--;
            }
        }
    }

    if (z != Q_fmt(1.0f))
    {
        y  = fxp_mul32_Q20(*(pt_table++), z);

        for (i = 7; i != 0; i--)
        {
            y += *(pt_table++);
            y  = fxp_mul32_Q20(y, z);
        }

        y += *(pt_table++);
    }
    else
    {
        y = 0;
    }

    return (y + (int_log2 << 20));
}

#endif

