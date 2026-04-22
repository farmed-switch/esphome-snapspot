

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO
#include    "aac_mem_funcs.h"
#include    "ps_stereo_processing.h"
#include    "ps_decorrelate.h"
#include    "ps_hybrid_synthesis.h"
#include    "ps_hybrid_analysis.h"
#include    "ps_applied.h"

void ps_applied(STRUCT_PS_DEC *h_ps_dec,
                Int32 rIntBufferLeft[][64],
                Int32 iIntBufferLeft[][64],
                Int32 *rIntBufferRight,
                Int32 *iIntBufferRight,
                Int32 scratch_mem[],
                Int32 band)

{

    ps_hybrid_analysis(rIntBufferLeft,
                       iIntBufferLeft,
                       h_ps_dec->mHybridRealLeft,
                       h_ps_dec->mHybridImagLeft,
                       h_ps_dec->hHybrid,
                       scratch_mem,
                       band);

    ps_decorrelate(h_ps_dec,
                   *rIntBufferLeft,
                   *iIntBufferLeft,
                   rIntBufferRight,
                   iIntBufferRight,
                   scratch_mem);

    ps_stereo_processing(h_ps_dec,
                         *rIntBufferLeft,
                         *iIntBufferLeft,
                         rIntBufferRight,
                         iIntBufferRight);

    ps_hybrid_synthesis((const Int32*)h_ps_dec->mHybridRealLeft,
                        (const Int32*)h_ps_dec->mHybridImagLeft,
                        *rIntBufferLeft,
                        *iIntBufferLeft,
                        h_ps_dec->hHybrid);

    ps_hybrid_synthesis((const Int32*)h_ps_dec->mHybridRealRight,
                        (const Int32*)h_ps_dec->mHybridImagRight,
                        rIntBufferRight,
                        iIntBufferRight,
                        h_ps_dec->hHybrid);

}
#endif

#endif

