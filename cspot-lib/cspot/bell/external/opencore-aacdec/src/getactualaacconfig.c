

#include "getactualaacconfig.h"
#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "ibstream.h"
#include "sfb.h"

#include "get_audio_specific_config.h"
#include "pvmp4audiodecoder_api.h"

#include "oscl_mem.h"
#include "e_elementid.h"
#include "e_sbr_element_id.h"
#include "get_dse.h"
#include "get_sbr_bitstream.h"
#include "get_prog_config.h"
#include "huffman.h"
#include "e_sbr_error.h"
#include "sbr_read_data.h"
#include "sbr_open.h"

#define ERROR_BUFFER_OVERRUN (-2)
#define KCODEC_INIT_FAILURE (-1)
#define LEFT (0)
#define RIGHT (1)

OSCL_EXPORT_REF Int32 GetActualAacConfig(UInt8* aConfigHeader,
        UInt8* aAudioObjectType,
        Int32* aConfigHeaderSize,
        UInt8* SamplingRateIndex,
        UInt32* NumChannels)
{

    tPVMP4AudioDecoderExternal * iAACDecExt = NULL;
    UInt           initialUsedBits;
    tDec_Int_File *pVars;
    MC_Info       *pMC_Info;

    Int            status = ERROR_BUFFER_OVERRUN;

    iAACDecExt = malloc(sizeof(tPVMP4AudioDecoderExternal));
    if (!iAACDecExt)
    {
        return 1;
    }
    iAACDecExt->inputBufferCurrentLength = 0;

    iAACDecExt->pInputBuffer = aConfigHeader;
    iAACDecExt->inputBufferMaxLength = PVMP4AUDIODECODER_INBUFSIZE;

    iAACDecExt->inputBufferUsedLength    = 0;
    iAACDecExt->remainderBits            = 0;

    Int32 memreq =  PVMP4AudioDecoderGetMemRequirements();

    UInt8 *pMem = calloc(memreq, sizeof(UInt8));

    if (pMem == 0)
    {
        return KCODEC_INIT_FAILURE;
    }

    if (PVMP4AudioDecoderInitLibrary(iAACDecExt, pMem) != 0)
    {
        return KCODEC_INIT_FAILURE;
    }

    iAACDecExt->inputBufferCurrentLength =  *aConfigHeaderSize;

    pVars = (tDec_Int_File *)pMem;

    pMC_Info = &pVars->mc_info;

    pVars->inputStream.pBuffer = iAACDecExt->pInputBuffer;

    pVars->inputStream.availableBits =
        (UInt)(iAACDecExt->inputBufferCurrentLength << INBUF_ARRAY_INDEX_SHIFT);

    initialUsedBits =
        (UInt)((iAACDecExt->inputBufferUsedLength << INBUF_ARRAY_INDEX_SHIFT) +
               iAACDecExt->remainderBits);

    pVars->inputStream.inputBufferCurrentLength =
        (UInt)iAACDecExt->inputBufferCurrentLength;

    pVars->inputStream.usedBits = initialUsedBits;

    pVars->aacPlusEnabled = TRUE;

    if (initialUsedBits <= pVars->inputStream.availableBits)
    {

        pVars->aacConfigUtilityEnabled = TRUE;

        status = get_audio_specific_config(pVars);

    }

    byte_align(&pVars->inputStream);

    *aConfigHeaderSize = (Int32)((pVars->inputStream.usedBits) >> 3);

    *SamplingRateIndex = pVars->prog_config.sampling_rate_idx;

    *NumChannels = pVars->mc_info.nch;

    if (pVars->mc_info.audioObjectType != pVars->mc_info.ExtendedAudioObjectType)
    {
        *aAudioObjectType = pVars->mc_info.ExtendedAudioObjectType;
    }
    else
    {
        *aAudioObjectType = pVars->mc_info.audioObjectType;
    }

    if (pVars->mc_info.sbrPresentFlag)
    {
        if (pVars->mc_info.psPresentFlag)
        {
            *NumChannels += 1;
        }
    }

    pVars->status = status;

    if (pMem != NULL)
    {
        free(pMem);
        pMem = NULL;
    }

    free(iAACDecExt);
    iAACDecExt = NULL;

    return status;
}
