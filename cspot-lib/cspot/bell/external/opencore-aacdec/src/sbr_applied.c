

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_applied.h"
#include    "sbr_read_data.h"

#include    "sbr_decode_envelope.h"
#include    "decode_noise_floorlevels.h"
#include    "sbr_requantize_envelope_data.h"
#include    "sbr_envelope_unmapping.h"
#include    "sbr_dec.h"
#include    "e_sbr_element_id.h"
#include    "aac_mem_funcs.h"

#ifdef PARAMETRICSTEREO
#include    "ps_bstr_decoding.h"
#include    "ps_allocate_decoder.h"

#endif

#include    "init_sbr_dec.h"

#define LEFT  (0)
#define RIGHT (1)

SBR_ERROR  sbr_applied(SBRDECODER_DATA * self,
                       SBRBITSTREAM * stream,
                       Int16 *ch_left,
                       Int16 *ch_right,
                       Int16 *timeData,
                       SBR_DEC *sbrDec,
                       tDec_Int_File  *pVars,
                       Int32 numChannels)
{
    SBR_ERROR err = SBRDEC_OK ;

    Int32 eleChannels = 0;

    SBR_CHANNEL *SbrChannel = self->SbrChannel;

    if (stream->NrElements)
    {

        err = sbr_read_data(self,
                            sbrDec,
                            stream);

        if (err != SBRDEC_OK)
        {

            self->SbrChannel[LEFT].syncState = UPSAMPLING;
            if (eleChannels == 2)
            {
                self->SbrChannel[RIGHT].syncState = UPSAMPLING;
            }
        }

        if (SbrChannel[LEFT].syncState == SBR_ACTIVE && self->setStreamType)
        {
            self->setStreamType = 0;

#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO

            Int sbrEnablePS = self->hParametricStereoDec->psDetected;

            pVars->mc_info.psPresentFlag  = sbrEnablePS;

            if (sbrEnablePS)
            {
                pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_PS;
                ps_allocate_decoder(self, 32);

                sbrDec->LC_aacP_DecoderFlag = OFF;
            }
            else
            {

                if (pVars->mc_info.ExtendedAudioObjectType != MP4AUDIO_PS)
                {
                    pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_SBR;

                    if (pVars->mc_info.nch > 1)
                    {
                        sbrDec->LC_aacP_DecoderFlag = ON;
                    }
                    else
                    {
                        sbrDec->LC_aacP_DecoderFlag = OFF;
                    }
                }
                else
                {
                    sbrEnablePS = 1;
                    pVars->mc_info.psPresentFlag  = sbrEnablePS;

                }
            }
#else

            pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_SBR;

            if (pVars->mc_info.nch > 1)
            {
                sbrDec->LC_aacP_DecoderFlag = ON;
            }
            else
            {
                sbrDec->LC_aacP_DecoderFlag = OFF;
            }
#endif

#else
            pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_SBR;

            sbrDec->LC_aacP_DecoderFlag = ON;

#endif

        }
        else
        {

            if (pVars->mc_info.ExtendedAudioObjectType == MP4AUDIO_AAC_LC)
            {

                pVars->mc_info.ExtendedAudioObjectType = MP4AUDIO_SBR;
            }

#ifdef HQ_SBR
            if (pVars->mc_info.nch > 1)
            {
                sbrDec->LC_aacP_DecoderFlag = ON;
            }
            else
            {
                sbrDec->LC_aacP_DecoderFlag = OFF;
            }
#else
            sbrDec->LC_aacP_DecoderFlag = ON;

#endif

            err = SBRDEC_OK;

        }

        eleChannels = (stream->sbrElement [LEFT].ElementID == SBR_ID_CPE) ? 2 : 1;

        if (SbrChannel[LEFT].syncState == SBR_ACTIVE)
        {

            sbr_decode_envelope(&(SbrChannel[LEFT].frameData));

            decode_noise_floorlevels(&(SbrChannel[LEFT].frameData));

            if (! SbrChannel[LEFT].frameData.coupling)
            {
                sbr_requantize_envelope_data(&(SbrChannel[LEFT].frameData));
            }

            if (eleChannels == 2)
            {

                sbr_decode_envelope(&(SbrChannel[RIGHT].frameData));

                decode_noise_floorlevels(&(SbrChannel[RIGHT].frameData));

                if (SbrChannel[RIGHT].frameData.coupling)
                {
                    sbr_envelope_unmapping(&(SbrChannel[ LEFT].frameData),
                                           &(SbrChannel[RIGHT].frameData));
                }
                else
                {
                    sbr_requantize_envelope_data(&(SbrChannel[RIGHT].frameData));
                }
            }
        }
        else
        {

            init_sbr_dec((sbrDec->outSampleRate >> 1),
                         pVars->mc_info.upsamplingFactor,
                         sbrDec,
                         &(self->SbrChannel[LEFT].frameData));

            if ((eleChannels == 2) && (SbrChannel[RIGHT].syncState != SBR_ACTIVE))
            {
                init_sbr_dec((sbrDec->outSampleRate >> 1),
                             pVars->mc_info.upsamplingFactor,
                             sbrDec,
                             &(self->SbrChannel[RIGHT].frameData));

            }

        }

    }

#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
    if (pVars->mc_info.ExtendedAudioObjectType == MP4AUDIO_PS)
    {
        ps_bstr_decoding(self->hParametricStereoDec);

        Int16 *tempInt16Ptr = (Int16 *)SbrChannel[RIGHT].frameData.V;
        self->hParametricStereoDec->R_ch_qmf_filter_history = (Int32 *)tempInt16Ptr;

        SbrChannel[LEFT].frameData.sbrQmfBufferReal = pVars->share.predictedSamples;
        SbrChannel[LEFT].frameData.sbrQmfBufferImag = &pVars->fxpCoef[0][920];

        sbr_dec(ch_left,
                timeData,
                &(SbrChannel[LEFT].frameData),
                (SbrChannel[LEFT].syncState == SBR_ACTIVE),
                sbrDec,
                &timeData[RIGHT],
                self->hParametricStereoDec,
                pVars);
    }
    else
    {
#endif
#endif

        SbrChannel[LEFT].frameData.sbrQmfBufferReal = pVars->fxpCoef[LEFT];
#ifdef HQ_SBR
        SbrChannel[LEFT].frameData.sbrQmfBufferImag = pVars->fxpCoef[RIGHT];
#endif

        sbr_dec(ch_left,
                timeData,
                &(SbrChannel[LEFT].frameData),
                (SbrChannel[LEFT].syncState == SBR_ACTIVE),
                sbrDec,
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
                NULL,
                NULL,
#endif
#endif
                pVars);

        if (numChannels == 2)
        {
            SbrChannel[RIGHT].frameData.sbrQmfBufferReal = pVars->fxpCoef[LEFT];
#ifdef HQ_SBR
            SbrChannel[RIGHT].frameData.sbrQmfBufferImag = pVars->fxpCoef[RIGHT];
#endif

            sbr_dec(ch_right,
                    &timeData[RIGHT],
                    &(SbrChannel[RIGHT].frameData),
                    (SbrChannel[RIGHT].syncState == SBR_ACTIVE),
                    sbrDec,
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
                    NULL,
                    NULL,
#endif
#endif
                    pVars);

        }

#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO
    }
#endif
#endif

    return err;
}

#endif

