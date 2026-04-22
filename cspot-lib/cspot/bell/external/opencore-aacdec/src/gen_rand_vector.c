

#include    "pv_audio_type_defs.h"
#include    "gen_rand_vector.h"
#include    "window_block_fxp.h"

#define     SQRT_OF_2       23170
#define     INV_SQRT_OF_2   11585
#define     INV_SQRT_POLY_ORDER     4

const UInt scale_mod_4[4] = { 16384, 19484, 23170, 27554};

const Int  inv_sqrt_coeff[INV_SQRT_POLY_ORDER+1] =
    { 4680, -17935, 27697, -22326, 11980};

Int gen_rand_vector(
    Int32     random_array[],
    const Int band_length,
    Int32*   pSeed,
    const Int power_scale)
{

    Int      k;
    UInt     power_adj;
    Int      q_adjust = 30;

    Int32    temp;
    Int32    seed;
    Int32    power;

    Int32*   pArray = &random_array[0];

    Int32    inv_sqrt_power;
    const Int  *pInvSqrtCoeff;

    power = 0;

    seed = *pSeed;

    if (band_length < 0 || band_length > LONG_WINDOW)
    {
        return  q_adjust;
    }

    for (k = (band_length >> 1); k != 0; k--)
    {

        seed *= 1664525L;
        seed += 1013904223L;

        temp =  seed >> 16;

        seed *= 1664525L;
        seed += 1013904223L;

        power  += ((temp * temp) >> 6);
        *pArray++ = temp;

        temp    = seed >> 16;
        power  += ((temp * temp) >> 6);
        *pArray++ = temp;

    }

    *pSeed = seed;

    k = 0;

    if (power)
    {

        while (power > 32767)
        {
            power >>= 1;
            k++;
        }

        k -= 13;

        power_adj = scale_mod_4[power_scale & 3];

        if (k < 0)
        {
            k = -k;
            if (k & 1)
            {
                power_adj = (UInt)(((UInt32) power_adj * SQRT_OF_2) >> 14);
            }
            q_adjust -= (k >> 1);
        }
        else if (k > 0)
        {
            if (k & 1)
            {
                power_adj = (UInt)(((UInt32) power_adj * INV_SQRT_OF_2) >> 14);
            }
            q_adjust += (k >> 1);
        }

        pInvSqrtCoeff = inv_sqrt_coeff;

        inv_sqrt_power  = (*(pInvSqrtCoeff++) * power) >> 15;
        inv_sqrt_power += *(pInvSqrtCoeff++);
        inv_sqrt_power  = (inv_sqrt_power * power) >> 15;
        inv_sqrt_power += *(pInvSqrtCoeff++);
        inv_sqrt_power  = (inv_sqrt_power * power) >> 15;
        inv_sqrt_power += *(pInvSqrtCoeff++);
        inv_sqrt_power  = (inv_sqrt_power * power) >> 15;
        inv_sqrt_power += *(pInvSqrtCoeff);

        inv_sqrt_power  = (inv_sqrt_power * power_adj) >> 13;

        pArray = &random_array[0];

        for (k = (band_length >> 1); k != 0; k--)
        {
            temp        = *(pArray) * inv_sqrt_power;
            *(pArray++) = temp;
            temp        = *(pArray) * inv_sqrt_power;
            *(pArray++) = temp;
        }

    }

    q_adjust  -= (power_scale >> 2);

    return (q_adjust);

}
