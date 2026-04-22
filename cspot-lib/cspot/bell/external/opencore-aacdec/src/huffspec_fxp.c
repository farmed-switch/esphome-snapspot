

#include    "pv_audio_type_defs.h"
#include    "aac_mem_funcs.h"
#include    "esc_iquant_scaling.h"
#include    "huffman.h"
#include    "unpack_idx.h"
#include    "pulse_nc.h"
#include    "iquant_table.h"
#include    "e_huffmanconst.h"

#include "pv_normalize.h"

#define ORDER        (3)

#define QTABLE       (27)

#define SIGNED32BITS  (31)

#define ROUND_UP (( ((UInt32) 1) << (QTABLE) )-1)

const UInt16 exptable[4] =
{
    0,
    19485,
    23171,
    27555

};

Int huffspec_fxp(
    FrameInfo *pFrameInfo,
    BITS      *pInputStream,
    Int       nsect,
    SectInfo  *pSectInfo,
    Int       factors[],
    Int32     coef[],
    Int16     quantSpec[],
    Int16     tmp_spec[],
    const FrameInfo  *pLongFrameInfo,
    PulseInfo  *pPulseInfo,
    Int         qFormat[])
{

    const Hcb       *pHcb;
    Int     i;
    Int     sfb;
    Int     idx_count;
    Int     sect_cb;
    Int     dim;
    Int     idx;
    Int     stop_idx;
    Int     sect_start;
    Int     sect_end;
    Int     *pSfbStart;
    Int     *pSfb;
    Int16     *pQuantSpec;
    Int     max = 0;

    Int     nsfb;
    Int     tot_sfb;
    Int     fac;

    Int32   *pCoef;
    UInt16     scale;

    Int     power_scale_div_4;
    Int     sfbWidth;

    void (*pUnpack_idx)(
        Int16  quant_spec[],
        Int  codeword_indx,
        const Hcb *pHuffCodebook,
        BITS  *pInputStream,
        Int *max);

    Int(*pDec_huff_tab)(BITS *) = NULL;

    UInt32 temp;
    Int    binaryDigits, QFormat;

    sect_start = 0;
    stop_idx   = 0;

    pSfbStart = pFrameInfo->frame_sfb_top;

    if (pSfbStart == NULL)
    {
        return (-1);
    }

    pSfb      = pSfbStart;

    for (i = nsect; i > 0; i--)
    {

        sect_cb  =  pSectInfo->sect_cb;
        if ((sect_cb > 15) || (sect_cb < 0))
        {
            return (-1);
        }
        sect_end =  pSectInfo->sect_end;

        if (sect_end < 0)
        {
            return (-1);
        }

        pSectInfo++;

        if (((sect_cb - 1) & 0xC) != 0xC)
        {

            if (sect_cb > BY4BOOKS)
            {
                dim = DIMENSION_2;
            }
            else
            {
                dim = DIMENSION_4;
            }

            pHcb        = &hcbbook_binary[sect_cb];

            if (sect_cb == ESCBOOK)
            {
                pUnpack_idx = &unpack_idx_esc;
            }
            else if (pHcb->signed_cb == FALSE)
            {
                pUnpack_idx = &unpack_idx_sgn;
            }
            else
            {
                pUnpack_idx = &unpack_idx;
            }

            switch (sect_cb)
            {
                case 1:
                    pDec_huff_tab = decode_huff_cw_tab1;
                    break;
                case 2:
                    pDec_huff_tab = decode_huff_cw_tab2;
                    break;
                case 3:
                    pDec_huff_tab = decode_huff_cw_tab3;
                    break;
                case 4:
                    pDec_huff_tab = decode_huff_cw_tab4;
                    break;
                case 5:
                    pDec_huff_tab = decode_huff_cw_tab5;
                    break;
                case 6:
                    pDec_huff_tab = decode_huff_cw_tab6;
                    break;
                case 7:
                    pDec_huff_tab = decode_huff_cw_tab7;
                    break;
                case 8:
                    pDec_huff_tab = decode_huff_cw_tab8;
                    break;
                case 9:
                    pDec_huff_tab = decode_huff_cw_tab9;
                    break;
                case 10:
                    pDec_huff_tab = decode_huff_cw_tab10;
                    break;
                case 11:
                    pDec_huff_tab = decode_huff_cw_tab11;
                    break;
                default:
                    return (-1);
            }

            pQuantSpec  = quantSpec + stop_idx;

            for (sfb = sect_start; sfb < sect_end; sfb++)
            {
                idx_count = *pSfb - stop_idx;
                stop_idx  = *pSfb++;

                while ((idx_count > 0) && (idx_count < 1024))
                {

                    idx = (*pDec_huff_tab)(pInputStream);

                    (*pUnpack_idx)(pQuantSpec,
                                   idx,
                                   pHcb,
                                   pInputStream,
                                   &max);

                    pQuantSpec += dim;
                    idx_count  -= dim;

                }

            }
        }
        else
        {

            pSfb        = pSfbStart + sect_end;

            idx_count   = *(pSfb - 1) - stop_idx;

            if ((idx_count > 1024) || (idx_count < 0))
            {
                return (-1);
            }

            pv_memset(&quantSpec[stop_idx],
                      0,
                      idx_count * sizeof(quantSpec[0]));

            pv_memset(&tmp_spec[stop_idx],
                      0,
                      idx_count * sizeof(tmp_spec[0]));

            stop_idx    = *(pSfb - 1);

        }

        sect_start = sect_end;

    }

    if (pFrameInfo->islong != FALSE)
    {
        if (pPulseInfo->pulse_data_present == 1)
        {
            pulse_nc(quantSpec,
                     pPulseInfo,
                     pLongFrameInfo,
                     &max);
        }

        pQuantSpec = quantSpec;

    }
    else
    {
        deinterleave(quantSpec,
                     tmp_spec,
                     pFrameInfo);

        pQuantSpec = tmp_spec;
    }

    if ((max < 0) || (max > 8192))
    {
        return (-1);
    }
    else
    {

        temp = inverseQuantTable[(max >> ORDER) + 1];
    }

    temp += ROUND_UP;

    temp >>= (QTABLE - 1);

    temp *= max;

    binaryDigits = 31 - pv_normalize(temp);

    if (binaryDigits < (SIGNED32BITS - QTABLE))
    {
        binaryDigits = SIGNED32BITS - QTABLE;
    }

    QFormat = SIGNED32BITS - binaryDigits;

    tot_sfb = 0;
    nsfb = pFrameInfo->sfb_per_win[0];
    pCoef = coef;

    for (i = pFrameInfo->num_win; i > 0; i--)
    {
        stop_idx  = 0;

        for (sfb = 0; sfb < nsfb; sfb++)
        {
            sfbWidth   =  pFrameInfo->win_sfb_top[0][sfb] - stop_idx;

            if ((sfbWidth < 0) || (sfbWidth > 1024))
            {
                return (-1);
            }

            stop_idx  += sfbWidth;

            fac   = factors[tot_sfb] - SF_OFFSET;
            scale = exptable[fac & 0x3];

            power_scale_div_4 = fac >> 2;

            power_scale_div_4++;

            qFormat[tot_sfb] = QFormat;

            esc_iquant_scaling(pQuantSpec,
                               pCoef,
                               sfbWidth,
                               QFormat,
                               scale,
                               max);

            pQuantSpec += sfbWidth;
            qFormat[tot_sfb] -= power_scale_div_4;
            pCoef += sfbWidth;

            tot_sfb++;

        }
    }

    return SUCCESS;

}
