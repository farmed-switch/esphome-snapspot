

#include "config.h"

#include "pv_audio_type_defs.h"
#include "iquant_table.h"
#include "esc_iquant_scaling.h"
#include "aac_mem_funcs.h"

#include "fxp_mul32.h"

#define ORDER        (3)

#define FACTOR       (2)

#define INPUTRANGE   (8192)

#define SPACING      (1<<ORDER)

#define TABLESIZE    (INPUTRANGE/SPACING)

#define QTABLE       (27)

#define SIGNED32BITS  (31)

#define ROUND_UP (( ((UInt32) 1) << (QTABLE) )-1)

#define     MASK_LOW16  0xffff
#define     UPPER16     16

#if ( defined(_ARM) || defined(_ARM_V4))

static inline Int32 abs2(Int32 x)
{
    Int32 z;

    __asm
    {
        sub  z, x, x, lsr #31
        eor  x, z, z, asr #31
    }
    return (x);
}

#define pv_abs(x)   abs2(x)

#elif (defined(PV_ARM_GCC_V5)||defined(PV_ARM_GCC_V4))

static inline Int32 abs2(Int32 x)
{
    register Int32 z;
    register Int32 y;
    register Int32 ra = x;
    asm volatile(
        "sub  %0, %2, %2, lsr #31\n\t"
        "eor  %1, %0, %0, asr #31"
    : "=&r*i"(z),
        "=&r*i"(y)
                : "r"(ra));

    return (y);
}

#define pv_abs(x)   abs2(x)

#else

#define pv_abs(x)   ((x) > 0)? (x) : (-x)

#endif

void esc_iquant_scaling(
    const Int16     quantSpec[],
    Int32         coef[],
    const Int     sfbWidth,
    Int const      QFormat,
    UInt16        scale,
    Int           maxInput)
{
    Int    i;
    Int    x;
    Int    y;
    Int    index;
    Int    shift;
    UInt   absX;
    UInt32 w1, w2;
    UInt32 deltaOneThird;
    UInt32 x1;
    UInt32 approxOneThird;
    Int32   mult_high;

#if ( defined(_ARM) || defined(_ARM_V4))

    {
        Int32   *temp;
        Int32   R12, R11, R10, R9;

        deltaOneThird = sizeof(Int32) * sfbWidth;
        temp = coef;

        __asm
        {
            MOV     R12, #0x0
            MOV     R11, #0x0
            MOV     R10, #0x0
            MOV     R9, #0x0
            SUBS    deltaOneThird, deltaOneThird, #0x20
loop:
            STMCSIA temp!, {R12, R11, R10, R9}
            STMCSIA temp!, {R12, R11, R10, R9}
            SUBCSS  deltaOneThird, deltaOneThird, #0x20
            BCS     loop

            MOVS    deltaOneThird, deltaOneThird, LSL #28
            STMCSIA temp!, {R12, R11, R10, R9}
            STMMIIA temp!, {R12, R11}
        }
    }

#else
    pv_memset(coef, 0, sizeof(Int32) * sfbWidth);
#endif

    if (maxInput > 0)
    {

        shift = QTABLE - QFormat;

        if (scale != 0)
        {
            if (maxInput < TABLESIZE)
            {

                for (i = sfbWidth - 1; i >= 0; i -= 4)
                {
                    x = quantSpec[i];
                    y = quantSpec[i-1];
                    if (x)
                    {
                        absX = pv_abs(x);
                        mult_high = (x * (inverseQuantTable[absX] >> shift));
                        coef[i] = fxp_mul32_by_16(mult_high, scale) << 1;
                    }

                    if (y)
                    {
                        absX = pv_abs(y);
                        mult_high = y * (inverseQuantTable[absX] >> shift);
                        coef[i-1] = fxp_mul32_by_16(mult_high, scale) << 1;
                    }

                    x = quantSpec[i-2];
                    y = quantSpec[i-3];
                    if (x)
                    {
                        absX = pv_abs(x);
                        mult_high = x * (inverseQuantTable[absX] >> shift);
                        coef[i-2] = fxp_mul32_by_16(mult_high, scale) << 1;
                    }

                    if (y)
                    {
                        absX = pv_abs(y);
                        mult_high = y * (inverseQuantTable[absX] >> shift);
                        coef[i-3] = fxp_mul32_by_16(mult_high, scale) << 1;
                    }
                }

            }

            else
            {
                for (i = sfbWidth - 1; i >= 0; i -= 4)
                {
                    x    = quantSpec[i];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i] = fxp_mul32_by_16(mult_high, scale) << 1;

                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i] = fxp_mul32_by_16(mult_high, scale) << 1;

                        }
                    }

                    x    = quantSpec[i-1];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = (x * (inverseQuantTable[absX] >> shift));
                            coef[i-1] = fxp_mul32_by_16(mult_high, scale) << 1;

                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-1] = fxp_mul32_by_16(mult_high, scale) << 1;
                        }
                    }

                    x    = quantSpec[i-2];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i-2] = fxp_mul32_by_16(mult_high, scale) << 1;
                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-2] = fxp_mul32_by_16(mult_high, scale) << 1;
                        }
                    }

                    x    = quantSpec[i-3];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i-3] = fxp_mul32_by_16(mult_high, scale) << 1;

                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-3] = fxp_mul32_by_16(mult_high, scale) << 1;

                        }
                    }

                }
            }
        }
        else
        {
            if (maxInput < TABLESIZE)
            {
                for (i = sfbWidth - 1; i >= 0; i -= 4)
                {
                    x = quantSpec[i];
                    y = quantSpec[i-1];
                    if (x)
                    {
                        absX = pv_abs(x);
                        mult_high = x * (inverseQuantTable[absX] >> shift);
                        coef[i] = mult_high >> 1;
                    }

                    if (y)
                    {
                        absX = pv_abs(y);
                        mult_high = y * (inverseQuantTable[absX] >> shift);
                        coef[i-1] = mult_high >> 1;
                    }

                    x = quantSpec[i-2];
                    y = quantSpec[i-3];
                    if (x)
                    {
                        absX = pv_abs(x);
                        mult_high = x * (inverseQuantTable[absX] >> shift);
                        coef[i-2] = mult_high >> 1;
                    }

                    if (y)
                    {
                        absX = pv_abs(y);
                        mult_high = y * (inverseQuantTable[absX] >> shift);
                        coef[i-3] = mult_high >> 1;
                    }
                }

            }

            else
            {
                for (i = sfbWidth - 1; i >= 0; i -= 4)
                {
                    x    = quantSpec[i];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i] = (mult_high >> 1);
                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i] = (mult_high >> 1);
                        }
                    }

                    x    = quantSpec[i-1];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i-1] = (mult_high >> 1);
                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-1] = (mult_high >> 1);
                        }
                    }

                    x    = quantSpec[i-2];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i-2] = (mult_high >> 1);
                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-2] = (mult_high >> 1);
                        }
                    }

                    x    = quantSpec[i-3];
                    if (x)
                    {
                        absX = pv_abs(x);
                        if (absX < TABLESIZE)
                        {
                            mult_high = x * (inverseQuantTable[absX] >> shift);
                            coef[i-3] = (mult_high >> 1);
                        }
                        else
                        {
                            index = absX >> ORDER;
                            w1 = inverseQuantTable[index];
                            w2 = inverseQuantTable[index+1];
                            approxOneThird = (w1 * FACTOR) >> shift;
                            x1 = index << ORDER;
                            deltaOneThird = (w2 - w1) * (absX - x1);
                            deltaOneThird >>= (shift + 2);
                            mult_high = x * (approxOneThird + deltaOneThird);
                            coef[i-3] = (mult_high >> 1);
                        }

                    }

                }

            }

        }

    }

}

