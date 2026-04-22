

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_decode_envelope.h"
#include    "sbr_constants.h"

void mapLowResEnergyVal(
    Int32  currVal,
    Int32 *prevData,
    Int32 offset,
    Int32 index,
    Int32 res);

Int32 indexLow2High(Int32 offset,
                    Int32 index,
                    Int32 res);

void sbr_decode_envelope(SBR_FRAME_DATA * hFrameData)

{
    Int32 i;
    Int32 no_of_bands;
    Int32 band;
    Int32 freqRes;
    Int32 *iEnvelope    = hFrameData->iEnvelope_man;
    Int32 *sfb_nrg_prev = hFrameData->sfb_nrg_prev_man;

    Int32  offset       = hFrameData->offset;
    Int32 *nSfb         = hFrameData->nSfb;
    Int32 *domain_vec   = hFrameData->domain_vec1;
    Int32 *frameInfo    = hFrameData->frameInfo;

    for (i = 0; i < frameInfo[0]; i++)
    {
        freqRes = frameInfo[frameInfo[0] + i + 2];
        no_of_bands = nSfb[freqRes];

        if (domain_vec[i] == 0)
        {
            mapLowResEnergyVal(*iEnvelope,
                               sfb_nrg_prev,
                               offset,
                               0,
                               freqRes);
            iEnvelope++;

            for (band = 1; band < no_of_bands; band++)
            {
                *iEnvelope +=  *(iEnvelope - 1);

                mapLowResEnergyVal(*iEnvelope,
                                   sfb_nrg_prev,
                                   offset,
                                   band,
                                   freqRes);
                iEnvelope++;
            }
        }
        else
        {
            for (band = 0; band < no_of_bands; band++)
            {
                *iEnvelope +=  sfb_nrg_prev[ indexLow2High(offset, band, freqRes)];

                mapLowResEnergyVal(*iEnvelope,
                                   sfb_nrg_prev,
                                   offset,
                                   band,
                                   freqRes);
                iEnvelope++;
            }
        }
    }
}

void mapLowResEnergyVal(
    Int32  currVal,
    Int32 *prevData,
    Int32  offset,
    Int32  index,
    Int32  res)
{
    Int32 tmp;

    if (res == LO)
    {
        if (offset >= 0)
        {
            if (index < offset)
            {
                prevData[index] = currVal;
            }
            else
            {
                tmp = (index << 1) - offset;
                prevData[tmp    ] = currVal;
                prevData[tmp +1 ] = currVal;
            }
        }
        else
        {
            offset = -offset;
            if (index < offset)
            {
                tmp = (index << 1) + index;
                prevData[tmp    ] = currVal;
                prevData[tmp + 1] = currVal;
                prevData[tmp + 2] = currVal;
            }
            else
            {
                tmp = (index << 1) + offset;
                prevData[tmp    ] = currVal;
                prevData[tmp + 1] = currVal;
            }
        }
    }
    else
    {
        prevData[index] = currVal;
    }
}

Int32 indexLow2High(Int32 offset,
                    Int32 index,
                    Int32 res)
{
    if (res == LO)
    {
        if (offset >= 0)
        {
            if (index < offset)
            {
                return(index);
            }
            else
            {
                return((index << 1) - offset);
            }
        }
        else
        {
            offset = -offset;
            if (index < offset)
            {
                return((index << 1) + index);
            }
            else
            {
                return((index << 1) + offset);
            }
        }
    }
    else
    {
        return(index);
    }
}

#endif

