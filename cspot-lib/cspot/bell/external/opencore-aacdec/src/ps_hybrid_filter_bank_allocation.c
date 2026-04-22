

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO
#include    "aac_mem_funcs.h"
#include    "ps_hybrid_filter_bank_allocation.h"
#include    "ps_all_pass_filter_coeff.h"

Int32 ps_hybrid_filter_bank_allocation(HYBRID **phHybrid,
                                       Int32 noBands,
                                       const Int32 *pResolution,
                                       Int32 **pPtr)
{
    Int32 i;
    Int32 tmp;
    Int32 maxNoChannels = 0;
    HYBRID *hs;
    Int32 *ptr = *pPtr;

    *phHybrid = (HYBRID *)NULL;

    hs = (HYBRID *)ptr;

    ptr += sizeof(HYBRID) / sizeof(*ptr);

    hs->pResolution = (Int32*)ptr;

    ptr += noBands * sizeof(Int32) / sizeof(*ptr);

    for (i = 0; i < noBands; i++)
    {

        hs->pResolution[i] = pResolution[i];

        if (pResolution[i] != HYBRID_8_CPLX &&
                pResolution[i] != HYBRID_2_REAL &&
                pResolution[i] != HYBRID_4_CPLX)
        {
            return 1;
        }

        if (pResolution[i] > maxNoChannels)
        {
            maxNoChannels = pResolution[i];
        }
    }

    hs->nQmfBands     = noBands;
    hs->qmfBufferMove = HYBRID_FILTER_LENGTH - 1;

    hs->mQmfBufferReal = (Int32 **)ptr;
    ptr += noBands * sizeof(ptr) / sizeof(*ptr);

    hs->mQmfBufferImag = (Int32 **)ptr;
    ptr += noBands * sizeof(ptr) / sizeof(*ptr);

    tmp = hs->qmfBufferMove;

    for (i = 0; i < noBands; i++)
    {
        hs->mQmfBufferReal[i] = ptr;
        ptr += tmp;

        hs->mQmfBufferImag[i] = ptr;
        ptr += tmp;

    }

    hs->mTempReal = ptr;
    ptr += maxNoChannels;

    hs->mTempImag = ptr;
    ptr += maxNoChannels;

    *phHybrid = hs;
    *pPtr = ptr;

    return 0;
}

#endif

#endif

