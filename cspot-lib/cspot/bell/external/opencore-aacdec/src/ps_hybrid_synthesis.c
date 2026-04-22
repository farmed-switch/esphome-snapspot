

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include "s_hybrid.h"
#include "ps_hybrid_synthesis.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

void ps_hybrid_synthesis(const Int32 *mHybridReal,
                         const Int32 *mHybridImag,
                         Int32 *mQmfReal,
                         Int32 *mQmfImag,
                         HYBRID *hHybrid)
{
    Int32  k;
    Int32  band;
    HYBRID_RES hybridRes;

    Int32 real;
    Int32 imag;
    Int32 *ptr_mQmfReal = mQmfReal;
    Int32 *ptr_mQmfImag = mQmfImag;
    const Int32 *ptr_mHybrid_Re = mHybridReal;
    const Int32 *ptr_mHybrid_Im = mHybridImag;

    for (band = 0; band < hHybrid->nQmfBands; band++)
    {
        hybridRes = (HYBRID_RES)(min(hHybrid->pResolution[band], 6) - 2);

        real  = *(ptr_mHybrid_Re++);
        real += *(ptr_mHybrid_Re++);
        imag  = *(ptr_mHybrid_Im++);
        imag += *(ptr_mHybrid_Im++);

        for (k = (hybridRes >> 1); k != 0; k--)
        {
            real += *(ptr_mHybrid_Re++);
            real += *(ptr_mHybrid_Re++);
            imag += *(ptr_mHybrid_Im++);
            imag += *(ptr_mHybrid_Im++);
        }

        *(ptr_mQmfReal++) = real;
        *(ptr_mQmfImag++) = imag;
    }
}

#endif

#endif

