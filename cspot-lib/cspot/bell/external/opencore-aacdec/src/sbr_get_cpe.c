

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_cpe.h"
#include    "buf_getbits.h"
#include    "extractframeinfo.h"
#include    "sbr_get_dir_control_data.h"
#include    "sbr_get_envelope.h"
#include    "sbr_get_noise_floor_data.h"
#include    "sbr_get_additional_data.h"
#include    "sbr_extract_extended_data.h"
#include    "aac_mem_funcs.h"

SBR_ERROR sbr_get_cpe(SBR_FRAME_DATA * hFrameDataLeft,
                      SBR_FRAME_DATA * hFrameDataRight,
                      BIT_BUFFER  * hBitBuf)
{
    Int32 i;
    Int32 bits;
    SBR_ERROR err =  SBRDEC_OK;

    bits = buf_getbits(hBitBuf, SI_SBR_RESERVED_PRESENT);

    if (bits)
    {
        buf_getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
        buf_getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
    }

    bits = buf_getbits(hBitBuf, SI_SBR_COUPLING_BITS);

    if (bits)
    {
        hFrameDataLeft->coupling = COUPLING_LEVEL;
        hFrameDataRight->coupling = COUPLING_BAL;
    }
    else
    {
        hFrameDataLeft->coupling = COUPLING_OFF;
        hFrameDataRight->coupling = COUPLING_OFF;
    }

    err = extractFrameInfo(hBitBuf, hFrameDataLeft);

    if (err != SBRDEC_OK)
    {
        return err;
    }

    if (hFrameDataLeft->coupling)
    {

        pv_memcpy(hFrameDataRight->frameInfo,
                  hFrameDataLeft->frameInfo,
                  LENGTH_FRAME_INFO * sizeof(Int32));

        hFrameDataRight->nNoiseFloorEnvelopes = hFrameDataLeft->nNoiseFloorEnvelopes;
        hFrameDataRight->frameClass = hFrameDataLeft->frameClass;

        sbr_get_dir_control_data(hFrameDataLeft, hBitBuf);
        sbr_get_dir_control_data(hFrameDataRight, hBitBuf);

        for (i = 0; i < hFrameDataLeft->nNfb; i++)
        {
            hFrameDataLeft->sbr_invf_mode_prev[i]  = hFrameDataLeft->sbr_invf_mode[i];
            hFrameDataRight->sbr_invf_mode_prev[i] = hFrameDataRight->sbr_invf_mode[i];

            hFrameDataLeft->sbr_invf_mode[i]  = (INVF_MODE) buf_getbits(hBitBuf, SI_SBR_INVF_MODE_BITS);
            hFrameDataRight->sbr_invf_mode[i] = hFrameDataLeft->sbr_invf_mode[i];
        }

        sbr_get_envelope(hFrameDataLeft, hBitBuf);
        sbr_get_noise_floor_data(hFrameDataLeft, hBitBuf);
        sbr_get_envelope(hFrameDataRight, hBitBuf);

    }
    else
    {
        err = extractFrameInfo(hBitBuf, hFrameDataRight);

        if (err != SBRDEC_OK)
        {
            return err;
        }

        sbr_get_dir_control_data(hFrameDataLeft,  hBitBuf);
        sbr_get_dir_control_data(hFrameDataRight, hBitBuf);

        for (i = 0; i <  hFrameDataLeft->nNfb; i++)
        {
            hFrameDataLeft->sbr_invf_mode_prev[i]  = hFrameDataLeft->sbr_invf_mode[i];
            hFrameDataLeft->sbr_invf_mode[i]  =
                (INVF_MODE) buf_getbits(hBitBuf, SI_SBR_INVF_MODE_BITS);
        }

        for (i = 0; i <  hFrameDataRight->nNfb; i++)
        {
            hFrameDataRight->sbr_invf_mode_prev[i] = hFrameDataRight->sbr_invf_mode[i];

            hFrameDataRight->sbr_invf_mode[i] =
                (INVF_MODE) buf_getbits(hBitBuf, SI_SBR_INVF_MODE_BITS);
        }
        sbr_get_envelope(hFrameDataLeft,  hBitBuf);
        sbr_get_envelope(hFrameDataRight, hBitBuf);

        sbr_get_noise_floor_data(hFrameDataLeft,  hBitBuf);

    }

    sbr_get_noise_floor_data(hFrameDataRight, hBitBuf);

    pv_memset((void *)hFrameDataLeft->addHarmonics,
              0,
              hFrameDataLeft->nSfb[HI]*sizeof(Int32));

    pv_memset((void *)hFrameDataRight->addHarmonics,
              0,
              hFrameDataRight->nSfb[HI]*sizeof(Int32));

    sbr_get_additional_data(hFrameDataLeft, hBitBuf);
    sbr_get_additional_data(hFrameDataRight, hBitBuf);

    sbr_extract_extended_data(hBitBuf
#ifdef PARAMETRICSTEREO
                              , NULL
#endif
                             );

    return SBRDEC_OK;

}

#endif

