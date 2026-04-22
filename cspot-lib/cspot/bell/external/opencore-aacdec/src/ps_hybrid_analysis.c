

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include    "s_hybrid.h"
#include    "aac_mem_funcs.h"
#include    "ps_channel_filtering.h"
#include    "ps_hybrid_analysis.h"

void ps_hybrid_analysis(const Int32 mQmfReal[][64],
                        const Int32 mQmfImag[][64],
                        Int32 *mHybridReal,
                        Int32 *mHybridImag,
                        HYBRID *pHybrid,
                        Int32 scratch_mem[],
                        Int32 i)

{

    Int32 band;
    HYBRID_RES hybridRes;
    Int32  chOffset = 0;

    Int32 *ptr_mHybrid_Re;
    Int32 *ptr_mHybrid_Im;

    Int32 *pt_mQmfBufferReal;
    Int32 *pt_mQmfBufferImag;

    pt_mQmfBufferReal = &scratch_mem[32 + i];

    for (band = 0; band < pHybrid->nQmfBands; band++)
    {
        pt_mQmfBufferImag = pt_mQmfBufferReal + 44;

        pt_mQmfBufferReal[HYBRID_FILTER_LENGTH_m_1] = mQmfReal[HYBRID_FILTER_DELAY][band];
        pt_mQmfBufferImag[HYBRID_FILTER_LENGTH_m_1] = mQmfImag[HYBRID_FILTER_DELAY][band];

        ptr_mHybrid_Re = &mHybridReal[ chOffset];
        ptr_mHybrid_Im = &mHybridImag[ chOffset];

        hybridRes = (HYBRID_RES)pHybrid->pResolution[band];
        switch (hybridRes)
        {

            case HYBRID_2_REAL:

                two_ch_filtering(pt_mQmfBufferReal,
                                 pt_mQmfBufferImag,
                                 ptr_mHybrid_Re,
                                 ptr_mHybrid_Im);
                chOffset += 2;

                break;

            case HYBRID_8_CPLX:

                eight_ch_filtering(pt_mQmfBufferReal,
                                   pt_mQmfBufferImag,
                                   pHybrid->mTempReal,
                                   pHybrid->mTempImag,
                                   scratch_mem);

                pv_memmove(ptr_mHybrid_Re, pHybrid->mTempReal, 4*sizeof(*pHybrid->mTempReal));

                ptr_mHybrid_Re += 2;

                *(ptr_mHybrid_Re++) +=  pHybrid->mTempReal[5];
                *(ptr_mHybrid_Re++) +=  pHybrid->mTempReal[4];
                *(ptr_mHybrid_Re++)  =  pHybrid->mTempReal[6];
                *(ptr_mHybrid_Re)  =  pHybrid->mTempReal[7];

                pv_memmove(ptr_mHybrid_Im, pHybrid->mTempImag, 4*sizeof(*pHybrid->mTempImag));
                ptr_mHybrid_Im += 2;

                *(ptr_mHybrid_Im++) +=  pHybrid->mTempImag[5];
                *(ptr_mHybrid_Im++) +=  pHybrid->mTempImag[4];
                *(ptr_mHybrid_Im++)  =  pHybrid->mTempImag[6];
                *(ptr_mHybrid_Im)  =  pHybrid->mTempImag[7];

                chOffset += 6;

                break;

            default:
                ;
        }

        pt_mQmfBufferReal = pt_mQmfBufferImag + 44;

    }

}
#endif

#endif

