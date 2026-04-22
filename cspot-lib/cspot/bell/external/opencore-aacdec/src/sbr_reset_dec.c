

#include    "config.h"
#ifdef AAC_PLUS

#include    "sbr_dec.h"

#include    "pv_log2.h"
#include    "fxp_mul32.h"

#include    "sbr_reset_dec.h"
#include    "sbr_find_start_andstop_band.h"
#include    "sbr_update_freq_scale.h"
#include    "sbr_downsample_lo_res.h"

SBR_ERROR sbr_reset_dec(SBR_FRAME_DATA * hFrameData,
                        SBR_DEC * sbrDec,
                        Int32 upsampleFac)
{

    SBR_ERROR err = SBRDEC_OK;
    Int lsbM;
    Int lsb;
    Int usb;
    Int32 i;
    Int32 tmp_q1;

    SBR_HEADER_DATA *headerData  = &(hFrameData->sbr_header);
    Int32           samplingFreq = sbrDec->outSampleRate;

    hFrameData->reset_flag = 1;

    err = sbr_find_start_andstop_band(samplingFreq,
                                      headerData->startFreq,
                                      headerData->stopFreq,
                                      &lsbM,
                                      &usb);

    if (err != SBRDEC_OK)
    {
        return err;
    }

    if (headerData->masterStatus == MASTER_RESET)
    {
        sbr_update_freq_scale(sbrDec->V_k_master,
                              &(sbrDec->Num_Master),
                              lsbM,
                              usb,
                              headerData->freqScale,
                              headerData->alterScale,
                              0);

    }

    sbrDec->NSfb[HI] = sbrDec->Num_Master - headerData->xover_band;

    for (i = headerData->xover_band; i <= sbrDec->Num_Master; i++)
    {
        sbrDec->FreqBandTable[HI][i-headerData->xover_band] = (Int)sbrDec->V_k_master[i];
    }

    if ((sbrDec->NSfb[HI] & 0x01) == 0)
    {

        sbrDec->NSfb[LO] = sbrDec->NSfb[HI] >> 1;

        for (i = 0; i <= sbrDec->NSfb[LO]; i++)
        {
            sbrDec->FreqBandTable[LO][i] = sbrDec->FreqBandTable[HI][(i<<1)];
        }
    }
    else
    {

        sbrDec->NSfb[LO] = (sbrDec->NSfb[HI] + 1) >> 1;

        sbrDec->FreqBandTable[LO][0] = sbrDec->FreqBandTable[HI][0];
        for (i = 1; i <= sbrDec->NSfb[LO]; i++)
        {
            sbrDec->FreqBandTable[LO][i] = sbrDec->FreqBandTable[HI][(i<<1)-1];
        }

    }

    lsb = sbrDec->FreqBandTable[LOW_RES][0];
    usb = sbrDec->FreqBandTable[LOW_RES][sbrDec->NSfb[LOW_RES]];

    sbrDec->lowSubband  = lsb;
    sbrDec->highSubband = usb;
    sbrDec->noSubbands  = usb - lsb;

    if ((lsb > 32) || (sbrDec->noSubbands <= 0))
    {
        return SBRDEC_ILLEGAL_SCFACTORS;
    }

    if (headerData->noise_bands == 0)
    {
        sbrDec->NoNoiseBands = 1;
    }
    else
    {

        if (! lsb)
        {
            return SBRDEC_ILLEGAL_SCFACTORS;
        }

        tmp_q1 = pv_log2((usb << 20) / lsb);

        tmp_q1 = fxp_mul32_Q15(headerData->noise_bands, tmp_q1);

        sbrDec->NoNoiseBands = (tmp_q1 + 16) >> 5;

        if (sbrDec->NoNoiseBands == 0)
        {
            sbrDec->NoNoiseBands = 1;
        }
    }

    headerData->noNoiseBands = sbrDec->NoNoiseBands;

    sbr_downsample_lo_res(sbrDec->FreqBandTableNoise,
                          sbrDec->NoNoiseBands,
                          sbrDec->FreqBandTable[LO],
                          sbrDec->NSfb[LO]);

    sbrDec->sbStopCodec = sbrDec->lowSubband;

    if (sbrDec->sbStopCodec > (upsampleFac << 5))
    {
        sbrDec->sbStopCodec = (upsampleFac << 5);
    }

    hFrameData->nSfb[LO] = sbrDec->NSfb[LO];
    hFrameData->nSfb[HI] = sbrDec->NSfb[HI];
    hFrameData->nNfb     = hFrameData->sbr_header.noNoiseBands;
    hFrameData->offset   = ((hFrameData->nSfb[LO]) << 1) - hFrameData->nSfb[HI];

    return (SBRDEC_OK);
}

#endif

