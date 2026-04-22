

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_noise_floor_data.h"
#include    "e_coupling_mode.h"
#include    "buf_getbits.h"
#include    "sbr_code_book_envlevel.h"
#include    "s_huffman.h"
#include    "sbr_decode_huff_cw.h"

void sbr_get_noise_floor_data(SBR_FRAME_DATA * h_frame_data,
                              BIT_BUFFER * hBitBuf)
{
    Int32 i;
    Int32 j;
    Int32 k;
    Int32 tmp;
    Int32 delta;
    Int32 noNoiseBands = h_frame_data->nNfb;
    Int32 envDataTableCompFactor;

    COUPLING_MODE coupling = h_frame_data->coupling;

    SbrHuffman hcb_noiseF;
    SbrHuffman hcb_noise;

    if (coupling == COUPLING_BAL)
    {
        hcb_noise  = bookSbrNoiseBalance11T;
        hcb_noiseF = bookSbrEnvBalance11F;
        envDataTableCompFactor = 1;
    }
    else
    {
        hcb_noise  = bookSbrNoiseLevel11T;
        hcb_noiseF = bookSbrEnvLevel11F;
        envDataTableCompFactor = 0;
    }

    h_frame_data->nNoiseFactors = h_frame_data->frameInfo[((h_frame_data->frameInfo[0]) << 1) + 3] * noNoiseBands;

    for (i = 0; i < h_frame_data->nNoiseFloorEnvelopes; i++)
    {
        k = i * noNoiseBands;
        if (h_frame_data->domain_vec2[i] == FREQ)
        {
            if (coupling == COUPLING_BAL)
            {
                tmp = buf_getbits(hBitBuf, SI_SBR_START_NOISE_BITS_BALANCE_AMP_RES_3_0) << 1;
                h_frame_data->sbrNoiseFloorLevel_man[k] = tmp;
                h_frame_data->sbrNoiseFloorLevel_exp[k] =   0;
            }
            else
            {
                tmp = buf_getbits(hBitBuf, SI_SBR_START_NOISE_BITS_AMP_RES_3_0);
                h_frame_data->sbrNoiseFloorLevel_man[k] = tmp;
                h_frame_data->sbrNoiseFloorLevel_exp[k] =   0;
            }

            for (j = 1; j < noNoiseBands; j++)
            {
                delta = sbr_decode_huff_cw(hcb_noiseF, hBitBuf);
                h_frame_data->sbrNoiseFloorLevel_man[k+j] = delta << envDataTableCompFactor;
                h_frame_data->sbrNoiseFloorLevel_exp[k+j] =   0;
            }
        }
        else
        {
            for (j = 0; j < noNoiseBands; j++)
            {
                delta = sbr_decode_huff_cw(hcb_noise, hBitBuf);
                h_frame_data->sbrNoiseFloorLevel_man[k+j] = delta << envDataTableCompFactor;
                h_frame_data->sbrNoiseFloorLevel_exp[k+j] =   0;
            }
        }
    }
}

#endif

