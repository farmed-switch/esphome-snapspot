

#include "pv_audio_type_defs.h"
#include "e_adif_const.h"

#include "s_progconfig.h"
#include "s_adif_header.h"
#include "s_bits.h"
#include "s_mc_info.h"
#include "s_frameinfo.h"
#include "s_tdec_int_file.h"

#include "get_prog_config.h"
#include "ibstream.h"

#include "get_adif_header.h"
#include "config.h"

#define ADIF_ID (0x41444946)

Int get_adif_header(
    tDec_Int_File *pVars,
    ProgConfig    *pScratchPCE)
{
    Int          i;
    UInt32       temp;
    Int          numConfigElementsMinus1;
    Int          bitStreamType;
    UInt32       theIDFromFile;

    BITS        *pInputStream = &pVars->inputStream;
    ADIF_Header *pHeader = &pVars->scratch.adif_header;
    Int          status  = SUCCESS;

    theIDFromFile = get17_n_lessbits((2 * LEN_BYTE), pInputStream);

    temp          = get17_n_lessbits((2 * LEN_BYTE), pInputStream);

    theIDFromFile = (theIDFromFile << (2 * LEN_BYTE)) | temp;

    if (theIDFromFile != ADIF_ID)
    {

        pInputStream->usedBits -= (4 * LEN_BYTE);

        status = -1;
    }
    else
    {

        temp =
            get1bits(
                pInputStream);

        if (temp != FALSE)
        {

            for (i = LEN_COPYRT_ID; i > 0; i--)
            {
                get9_n_lessbits(LEN_BYTE,
                                pInputStream);
            }

        }

        get9_n_lessbits(
            LEN_ORIG + LEN_HOME,
            pInputStream);

        bitStreamType =
            get1bits(
                pInputStream);

        pHeader->bitrate =
            getbits(
                LEN_BIT_RATE,
                pInputStream);

        numConfigElementsMinus1 =  get9_n_lessbits(LEN_NUM_PCE,
                                   pInputStream);

        for (i = numConfigElementsMinus1;
                (i >= 0) && (status == SUCCESS);
                i--)
        {

            if (bitStreamType == CONSTANT_RATE_BITSTREAM)
            {
                getbits(
                    LEN_ADIF_BF,
                    pInputStream);
            }

            pVars->adif_test = 1;

            status =
                get_prog_config(
                    pVars,
                    pScratchPCE);

#ifdef AAC_PLUS

            if ((pVars->prog_config.sampling_rate_idx >= 6) && (pVars->aacPlusEnabled == TRUE) &&
                    pVars->mc_info.audioObjectType == MP4AUDIO_AAC_LC)
            {
                pVars->mc_info.upsamplingFactor = 2;
                pVars->prog_config.sampling_rate_idx -= 3;
                pVars->mc_info.sbrPresentFlag = 1;
                pVars->sbrDecoderData.SbrChannel[0].syncState = UPSAMPLING;
                pVars->sbrDecoderData.SbrChannel[1].syncState = UPSAMPLING;
            }
#endif

        }

    }

    return status;

}
