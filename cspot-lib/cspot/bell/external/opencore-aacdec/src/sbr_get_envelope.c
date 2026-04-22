

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_envelope.h"
#include    "s_huffman.h"
#include    "e_coupling_mode.h"
#include    "sbr_code_book_envlevel.h"
#include    "buf_getbits.h"
#include    "sbr_decode_huff_cw.h"

void sbr_get_envelope(SBR_FRAME_DATA * h_frame_data,
                      BIT_BUFFER * hBitBuf)
{
    Int32   i;
    Int32   j;
    Int32   tmp;
    Int32   no_band[MAX_ENVELOPES];
    Int32   delta = 0;
    Int32   offset = 0;
    Int32   ampRes;
    Int32   envDataTableCompFactor;
    Int32   start_bits;
    Int32   start_bits_balance;
    SbrHuffman    hcb_t;
    SbrHuffman    hcb_f;
    COUPLING_MODE coupling = h_frame_data->coupling;

    h_frame_data->nScaleFactors = 0;

    if ((h_frame_data->frameClass   == FIXFIX) &&
            (h_frame_data->frameInfo[0] == 1))
    {
        h_frame_data->ampRes = SBR_AMP_RES_1_5;
    }
    else
    {
        h_frame_data->ampRes = h_frame_data->sbr_header.ampResolution;
    }

    ampRes = h_frame_data->ampRes;

    if (ampRes == SBR_AMP_RES_3_0)
    {
        start_bits         = SI_SBR_START_ENV_BITS_AMP_RES_3_0;
        start_bits_balance = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_3_0;
    }
    else
    {
        start_bits         = SI_SBR_START_ENV_BITS_AMP_RES_1_5;
        start_bits_balance = SI_SBR_START_ENV_BITS_BALANCE_AMP_RES_1_5;
    }

    for (i = 0; i < h_frame_data->frameInfo[0]; i++)
    {
        no_band[i] =
            h_frame_data->nSfb[h_frame_data->frameInfo[h_frame_data->frameInfo[0] + 2 + i]];
        h_frame_data->nScaleFactors += no_band[i];
    }

    if (coupling == COUPLING_BAL)
    {
        envDataTableCompFactor = 1;
        if (ampRes == SBR_AMP_RES_1_5)
        {
            hcb_t = bookSbrEnvBalance10T;
            hcb_f = bookSbrEnvBalance10F;
        }
        else
        {
            hcb_t = bookSbrEnvBalance11T;
            hcb_f = bookSbrEnvBalance11F;
        }
    }
    else
    {
        envDataTableCompFactor = 0;
        if (ampRes == SBR_AMP_RES_1_5)
        {
            hcb_t = bookSbrEnvLevel10T;
            hcb_f = bookSbrEnvLevel10F;
        }
        else
        {
            hcb_t = bookSbrEnvLevel11T;
            hcb_f = bookSbrEnvLevel11F;
        }
    }

    for (j = 0; j < h_frame_data->frameInfo[0]; j++)
    {
        if (h_frame_data->domain_vec1[j] == FREQ)
        {
            if (coupling == COUPLING_BAL)
            {
                tmp = buf_getbits(hBitBuf, start_bits_balance);
                h_frame_data->iEnvelope_man[offset] = tmp << envDataTableCompFactor;
            }
            else
            {
                tmp = buf_getbits(hBitBuf, start_bits);
                h_frame_data->iEnvelope_man[offset] = tmp;
            }
        }

        for (i = (1 - h_frame_data->domain_vec1[j]); i < no_band[j]; i++)
        {

            if (h_frame_data->domain_vec1[j] == FREQ)
            {
                delta = sbr_decode_huff_cw(hcb_f, hBitBuf);
            }
            else
            {
                delta = sbr_decode_huff_cw(hcb_t, hBitBuf);
            }

            h_frame_data->iEnvelope_man[offset + i] = delta << envDataTableCompFactor;
        }
        offset += no_band[j];
    }

}

#endif

