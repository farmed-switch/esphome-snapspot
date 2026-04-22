

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include "pv_audio_type_defs.h"
#include "fxp_mul32.h"
#include "pv_sine.h"

#define R_SHIFT     30

#define Q_fmt(x)   (Int32)(x*((Int32)1<<R_SHIFT) + (x>=0?0.5F:-0.5F))

const Int32 sin_table[9] =
{
    Q_fmt(0.00001724684028), Q_fmt(-0.00024606242846),
    Q_fmt(0.00007297328923), Q_fmt(0.00826706596417),
    Q_fmt(0.00003585160465), Q_fmt(-0.16667772526248),
    Q_fmt(0.00000174197440), Q_fmt(0.99999989138797),
    Q_fmt(0.00000000110513)
};

Int32 pv_sine(Int32 z)
{
    Int32 sine;
    Int32 i;
    const Int32 *pt_table = sin_table;
    Int32 sign = 0;

    if (z < 0)
    {
        z = -z;
        sign = 1;
    }

    if (z > Q_fmt(0.0015))
    {
        sine  = fxp_mul32_Q30(*(pt_table++), z);

        for (i = 7; i != 0; i--)
        {
            sine += *(pt_table++);
            sine  = fxp_mul32_Q30(sine, z);
        }

    }
    else
    {
        sine = z;
    }

    if (sign)
    {
        sine = -sine;
    }

    return sine;
}

Int32 pv_cosine(Int32 z)
{
    Int32 cosine;

    if (z < 0)
    {
        z = -z;
    }

    if (z > Q_fmt(0.0015))
    {
        z = Q_fmt(1.57079632679490) - z;

        cosine  = pv_sine(z);
    }
    else
    {
        cosine = Q_fmt(0.99999999906868) - (fxp_mul32_Q30(z, z) >> 1);
    }

    return cosine;
}

#endif

#endif

