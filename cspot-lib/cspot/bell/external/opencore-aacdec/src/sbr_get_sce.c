

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_sce.h"
#include    "sbr_get_additional_data.h"
#include    "sbr_extract_extended_data.h"
#include    "buf_getbits.h"
#include    "sbr_get_envelope.h"
#include    "sbr_get_noise_floor_data.h"
#include    "extractframeinfo.h"
#include    "sbr_get_dir_control_data.h"
#include    "e_invf_mode.h"
#include    "aac_mem_funcs.h"

SBR_ERROR sbr_get_sce(SBR_FRAME_DATA * hFrameData,
                      BIT_BUFFER * hBitBuf
#ifdef PARAMETRICSTEREO
                      , HANDLE_PS_DEC hParametricStereoDec
#endif
                     )
{
    Int32 i;
    Int32 bits;
    SBR_ERROR err =  SBRDEC_OK;

    bits = buf_getbits(hBitBuf, SI_SBR_RESERVED_PRESENT);

    if (bits)
    {
        buf_getbits(hBitBuf, SI_SBR_RESERVED_BITS_DATA);
    }

    err = extractFrameInfo(hBitBuf, hFrameData);

    if (err != SBRDEC_OK)
    {
        return err;
    }

    sbr_get_dir_control_data(hFrameData, hBitBuf);

    for (i = 0; i < hFrameData->nNfb; i++)
    {
        hFrameData->sbr_invf_mode_prev[i] = hFrameData->sbr_invf_mode[i];
        hFrameData->sbr_invf_mode[i] =
            (INVF_MODE) buf_getbits(hBitBuf, SI_SBR_INVF_MODE_BITS);
    }

    sbr_get_envelope(hFrameData, hBitBuf);

    sbr_get_noise_floor_data(hFrameData, hBitBuf);

    pv_memset((void *)hFrameData->addHarmonics,
              0,
              hFrameData->nSfb[HI]*sizeof(Int32));

    sbr_get_additional_data(hFrameData, hBitBuf);

    sbr_extract_extended_data(hBitBuf
#ifdef PARAMETRICSTEREO
                              , hParametricStereoDec
#endif
                             );

    hFrameData->coupling = COUPLING_OFF;

    return SBRDEC_OK;

}

#endif
