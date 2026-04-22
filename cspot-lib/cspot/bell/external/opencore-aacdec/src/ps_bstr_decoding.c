

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include "pv_audio_type_defs.h"
#include "aac_mem_funcs.h"
#include "ps_bstr_decoding.h"
#include "ps_decode_bs_utils.h"

const Int32 aNoIidBins[3] = {NO_LOW_RES_IID_BINS, NO_IID_BINS, NO_HI_RES_BINS};
const Int32 aNoIccBins[3] = {NO_LOW_RES_ICC_BINS, NO_ICC_BINS, NO_HI_RES_BINS};
const Int32 aFixNoEnvDecode[4] = {0, 1, 2, 4};

void ps_bstr_decoding(STRUCT_PS_DEC *ps_dec)
{
    UInt32 env;
    Int32 noIidSteps;

    if (!ps_dec->bPsDataAvail)
    {
        ps_dec->noEnv = 0;
    }

    noIidSteps = ps_dec->bFineIidQ ? NO_IID_STEPS_FINE : NO_IID_STEPS;

    for (env = 0; env < ps_dec->noEnv; env++)
    {
        Int32 *aPrevIidIndex;
        Int32 *aPrevIccIndex;
        if (env == 0)
        {
            aPrevIidIndex = ps_dec->aIidPrevFrameIndex;
            aPrevIccIndex = ps_dec->aIccPrevFrameIndex;
        }
        else
        {
            aPrevIidIndex = ps_dec->aaIidIndex[env-1];
            aPrevIccIndex = ps_dec->aaIccIndex[env-1];
        }

        differential_Decoding(ps_dec->bEnableIid,
                              ps_dec->aaIidIndex[env],
                              aPrevIidIndex,
                              ps_dec->abIidDtFlag[env],
                              aNoIidBins[ps_dec->freqResIid],
                              (ps_dec->freqResIid) ? 1 : 2,
                              -noIidSteps,
                              noIidSteps);

        differential_Decoding(ps_dec->bEnableIcc,
                              ps_dec->aaIccIndex[env],
                              aPrevIccIndex,
                              ps_dec->abIccDtFlag[env],
                              aNoIccBins[ps_dec->freqResIcc],
                              (ps_dec->freqResIcc) ? 1 : 2,
                              0,
                              NO_ICC_STEPS - 1);

    }

    if (ps_dec->noEnv == 0)
    {
        ps_dec->noEnv = 1;

        if (ps_dec->bEnableIid)
        {
            pv_memmove(ps_dec->aaIidIndex[ps_dec->noEnv-1],
                       ps_dec->aIidPrevFrameIndex,
                       NO_HI_RES_BINS*sizeof(*ps_dec->aIidPrevFrameIndex));

        }
        else
        {
            pv_memset((void *)ps_dec->aaIidIndex[ps_dec->noEnv-1],
                      0,
                      NO_HI_RES_BINS*sizeof(**ps_dec->aaIidIndex));
        }
        if (ps_dec->bEnableIcc)
        {
            pv_memmove(ps_dec->aaIccIndex[ps_dec->noEnv-1],
                       ps_dec->aIccPrevFrameIndex,
                       NO_HI_RES_BINS*sizeof(*ps_dec->aIccPrevFrameIndex));
        }
        else
        {
            pv_memset((void *)ps_dec->aaIccIndex[ps_dec->noEnv-1],
                      0,
                      NO_HI_RES_BINS*sizeof(**ps_dec->aaIccIndex));
        }
    }

    pv_memmove(ps_dec->aIidPrevFrameIndex,
               ps_dec->aaIidIndex[ps_dec->noEnv-1],
               NO_HI_RES_BINS*sizeof(*ps_dec->aIidPrevFrameIndex));

    pv_memmove(ps_dec->aIccPrevFrameIndex,
               ps_dec->aaIccIndex[ps_dec->noEnv-1],
               NO_HI_RES_BINS*sizeof(*ps_dec->aIccPrevFrameIndex));

    ps_dec->bPsDataAvail = 0;

    if (ps_dec->bFrameClass == 0)
    {
        Int32 shift;

        shift = ps_dec->noEnv >> 1;

        ps_dec->aEnvStartStop[0] = 0;

        for (env = 1; env < ps_dec->noEnv; env++)
        {
            ps_dec->aEnvStartStop[env] =
                (env * ps_dec->noSubSamples) >> shift;
        }

        ps_dec->aEnvStartStop[ps_dec->noEnv] = ps_dec->noSubSamples;
    }
    else
    {
        ps_dec->aEnvStartStop[0] = 0;

        if (ps_dec->aEnvStartStop[ps_dec->noEnv] < ps_dec->noSubSamples)
        {
            ps_dec->noEnv++;
            ps_dec->aEnvStartStop[ps_dec->noEnv] = ps_dec->noSubSamples;

            pv_memmove(ps_dec->aaIidIndex[ps_dec->noEnv],
                       ps_dec->aaIidIndex[ps_dec->noEnv-1],
                       NO_HI_RES_BINS*sizeof(**ps_dec->aaIidIndex));

            pv_memmove(ps_dec->aaIccIndex[ps_dec->noEnv],
                       ps_dec->aaIccIndex[ps_dec->noEnv-1],
                       NO_HI_RES_BINS*sizeof(**ps_dec->aaIccIndex));
        }

        for (env = 1; env < ps_dec->noEnv; env++)
        {
            UInt32 thr;
            thr = ps_dec->noSubSamples - ps_dec->noEnv + env;

            if (ps_dec->aEnvStartStop[env] > thr)
            {
                ps_dec->aEnvStartStop[env] = thr;
            }
            else
            {
                thr = ps_dec->aEnvStartStop[env-1] + 1;

                if (ps_dec->aEnvStartStop[env] < thr)
                {
                    ps_dec->aEnvStartStop[env] = thr;
                }
            }
        }
    }

    for (env = 0; env < ps_dec->noEnv; env++)
    {
        if (ps_dec->freqResIid == 2)
        {
            map34IndexTo20(ps_dec->aaIidIndex[env]);
        }
        if (ps_dec->freqResIcc == 2)
        {
            map34IndexTo20(ps_dec->aaIccIndex[env]);
        }
    }

}

#endif

#endif

