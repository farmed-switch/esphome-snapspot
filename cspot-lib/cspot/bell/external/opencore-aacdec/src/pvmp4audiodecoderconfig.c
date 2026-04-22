

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "ibstream.h"
#include "sfb.h"

#include "get_audio_specific_config.h"
#include "pvmp4audiodecoder_api.h"
#include "config.h"

OSCL_EXPORT_REF Int PVMP4AudioDecoderConfig(
    tPVMP4AudioDecoderExternal  *pExt,
    void                        *pMem)
{

    UInt           initialUsedBits;
    tDec_Int_File *pVars;

    Int            status = MP4AUDEC_INCOMPLETE_FRAME;

    pVars = (tDec_Int_File *)pMem;

    pVars->inputStream.pBuffer = pExt->pInputBuffer;

    pVars->inputStream.inputBufferCurrentLength =
        (UInt)pExt->inputBufferCurrentLength;

    pVars->inputStream.availableBits =
        (UInt)(pExt->inputBufferCurrentLength << INBUF_ARRAY_INDEX_SHIFT);

    initialUsedBits =
        (UInt)((pExt->inputBufferUsedLength << INBUF_ARRAY_INDEX_SHIFT) +
               pExt->remainderBits);

    pVars->inputStream.usedBits = initialUsedBits;

    if (initialUsedBits <= pVars->inputStream.availableBits)
    {

        pVars->aacConfigUtilityEnabled = FALSE;

        status = get_audio_specific_config(pVars);

    }

    byte_align(&pVars->inputStream);

    if (status == SUCCESS)
    {

        pVars->bno++;

        pExt->samplingRate =
            samp_rate_info[pVars->prog_config.sampling_rate_idx].samp_rate;

        pExt->aacPlusEnabled = pVars->aacPlusEnabled;

        pExt->encodedChannels = 2;

        pExt->frameLength = pVars->frameLength;
#ifdef AAC_PLUS
        pExt->aacPlusUpsamplingFactor = pVars->mc_info.upsamplingFactor;
#endif

    }
    else
    {

        status = MP4AUDEC_INVALID_FRAME;

        if (pVars->inputStream.usedBits > pVars->inputStream.availableBits)
        {

            pVars->inputStream.usedBits = pVars->inputStream.availableBits;

            status = MP4AUDEC_INCOMPLETE_FRAME;
        }

    }

    pExt->inputBufferUsedLength =
        pVars->inputStream.usedBits >> INBUF_ARRAY_INDEX_SHIFT;

    pExt->remainderBits = pVars->inputStream.usedBits & INBUF_BIT_MODULO_MASK;

    pVars->status = status;

    return (status);

}

