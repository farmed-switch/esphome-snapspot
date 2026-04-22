

#include "config.h"

#include "pv_audio_type_defs.h"
#include "s_hcb.h"
#include "ibstream.h"
#include "unpack_idx.h"

#include "fxp_mul32.h"

#define DIV_3_CUBED    19
#define THREE_CUBED    27

#define DIV_3_SQUARED  57
#define THREE_SQUARED   9

#define Q_FORMAT_MOD   13
#define Q_FORMAT_MOD2   9
#define Q_FORMAT_MOD3   9

#define LOWER_5_BITS_MASK 0x1F

#if ( defined(PV_ARM_V5) || defined(PV_ARM_V4))

static inline Int32 abs1(Int32 x)
{
    Int32 z;

    __asm
    {
        sub  z, x, x, lsr #31
        eor  x, z, z, asr #31
    }
    return (x);
}

#define pv_abs(x)   abs1(x)

#else

#define pv_abs(x)   ((x) > 0)? (x) : (-x)

#endif

const Int div_mod[18] =
{

    0xCC,
    0xCC,
    0xCC,
    2731,
    0xCC,
    0xCC,
    0xCC,
    0xCC,
    1025,
    911,
    0xCC,
    0xCC,
    0xCC,
    631,
    0xCC,
    0xCC,
    0xCC,
    482,
};

void unpack_idx(
    Int16   quant_spec[],
    Int codeword_indx,
    const Hcb   *pHuffCodebook,
    BITS  *pInputStream,
    Int *max)
{
    Int16 *pQuantSpec = &quant_spec[0];
    Int  temp_spec;

    const Int mod = pHuffCodebook->mod;
    const Int off = pHuffCodebook->off;

    OSCL_UNUSED_ARG(pInputStream);

    if (pHuffCodebook->dim == DIMENSION_4)
    {

        temp_spec      = (codeword_indx * DIV_3_CUBED) >> Q_FORMAT_MOD3;

        codeword_indx -= temp_spec * THREE_CUBED;

        temp_spec -= off;
        *pQuantSpec++  = (Int16)temp_spec;

        temp_spec = pv_abs(temp_spec);

        if (temp_spec > *max)
        {
            *max = temp_spec;
        }

        temp_spec      = (codeword_indx * DIV_3_SQUARED) >> Q_FORMAT_MOD2;

        codeword_indx -= temp_spec * THREE_SQUARED;

        temp_spec -= off;
        *pQuantSpec++  = (Int16)temp_spec;

        temp_spec = pv_abs(temp_spec);

        if (temp_spec > *max)
        {
            *max = temp_spec;
        }
    }

    temp_spec      = ((Int32) codeword_indx * div_mod[mod]) >> Q_FORMAT_MOD;

    codeword_indx -= temp_spec * mod;

    temp_spec -= off;
    *pQuantSpec++  = (Int16)temp_spec;

    temp_spec = pv_abs(temp_spec);

    if (temp_spec > *max)
    {
        *max = temp_spec;
    }

    codeword_indx -= off;
    *pQuantSpec    = (Int16)codeword_indx ;

    codeword_indx = pv_abs(codeword_indx);

    if (codeword_indx > *max)
    {
        *max = codeword_indx;
    }

    return ;
}

void unpack_idx_sgn(
    Int16   quant_spec[],
    Int codeword_indx,
    const Hcb   *pHuffCodebook,
    BITS  *pInputStream,
    Int *max)
{
    Int16 *pQuantSpec = &quant_spec[0];
    Int  temp_spec;
    Int  sgn;

    const Int mod = pHuffCodebook->mod;
    const Int off = pHuffCodebook->off;

    if (pHuffCodebook->dim == DIMENSION_4)
    {

        preload_cache((Int32 *)pQuantSpec);
        temp_spec      = (codeword_indx * DIV_3_CUBED) >> Q_FORMAT_MOD3;

        codeword_indx -= temp_spec * THREE_CUBED;

        temp_spec -= off;
        if (temp_spec)
        {
            sgn = get1bits(pInputStream);

            *pQuantSpec++ = (Int16)((sgn) ? -temp_spec : temp_spec);

            temp_spec = pv_abs(temp_spec);

            if (temp_spec > *max)
            {
                *max = temp_spec;
            }

        }
        else
        {
            *pQuantSpec++ = 0;
        }

        temp_spec      = (codeword_indx * DIV_3_SQUARED) >> Q_FORMAT_MOD2;

        codeword_indx -= temp_spec * THREE_SQUARED;

        temp_spec -= off;
        if (temp_spec)
        {

            sgn = get1bits(pInputStream);

            *pQuantSpec++ = (Int16)((sgn) ? -temp_spec : temp_spec);

            temp_spec = pv_abs(temp_spec);

            if (temp_spec > *max)
            {
                *max = temp_spec;
            }
        }
        else
        {
            *pQuantSpec++ = 0;
        }
    }

    temp_spec      = ((Int32) codeword_indx * div_mod[mod]) >> Q_FORMAT_MOD;

    codeword_indx -= temp_spec * mod;

    temp_spec -= off;
    if (temp_spec)
    {

        sgn = get1bits(pInputStream);

        *pQuantSpec++ = (Int16)((sgn) ? -temp_spec : temp_spec);

        temp_spec = pv_abs(temp_spec);

        if (temp_spec > *max)
        {
            *max = temp_spec;
        }
    }
    else
    {
        *pQuantSpec++ = 0;
    }

    codeword_indx -= off;
    if (codeword_indx)
    {

        sgn = get1bits(pInputStream);

        *pQuantSpec = (Int16)((sgn) ? -codeword_indx : codeword_indx);

        codeword_indx = pv_abs(codeword_indx);

        if (codeword_indx > *max)
        {
            *max = codeword_indx;
        }
    }
    else
    {
        *pQuantSpec = 0;
    }

    return ;
}

void unpack_idx_esc(
    Int16   quant_spec[],
    Int codeword_indx,
    const Hcb   *pHuffCodebook,
    BITS  *pInputStream,
    Int *max)
{
    Int  temp_spec;
    Int  sgn1 = 0, sgn2 = 0;
    Int N;
    Int32 esc_seq;

    const Int mod = pHuffCodebook->mod;
    const Int off = pHuffCodebook->off;

    temp_spec      = ((Int32) codeword_indx * div_mod[mod]) >> Q_FORMAT_MOD;

    codeword_indx -= temp_spec * mod;

    temp_spec -= off;
    if (temp_spec)
    {
        sgn1 = get1bits(pInputStream);
    }

    codeword_indx -= off;
    if (codeword_indx)
    {
        sgn2 = get1bits(pInputStream);
    }

    if ((temp_spec & LOWER_5_BITS_MASK) == 16)
    {
        N = 3;
        do
        {
            N++;

            esc_seq = get1bits(pInputStream);

        }
        while (esc_seq != 0);

        esc_seq  = getbits(N, pInputStream);

        esc_seq += (1 << N);

        temp_spec = (Int)((temp_spec * esc_seq) >> 4);

    }

    if (sgn1)
    {
        quant_spec[0]  = (Int16)(-temp_spec);
    }
    else
    {
        quant_spec[0]  = (Int16)temp_spec;
    }

    temp_spec = pv_abs(temp_spec);

    if (temp_spec > *max)
    {
        *max = temp_spec;
    }

    if ((codeword_indx & LOWER_5_BITS_MASK) == 16)
    {
        N = 3;
        do
        {
            N++;

            esc_seq = get1bits(pInputStream);

        }
        while (esc_seq != 0);

        esc_seq  = getbits(N, pInputStream);

        esc_seq += (1 << N);

        codeword_indx = (Int)((codeword_indx * esc_seq) >> 4);
    }

    if (sgn2)
    {
        quant_spec[1]    = (Int16)(-codeword_indx);
    }
    else
    {
        quant_spec[1]    = (Int16)codeword_indx;
    }

    codeword_indx = pv_abs(codeword_indx);

    if (codeword_indx > *max)
    {
        *max = codeword_indx;
    }

    return ;
}
