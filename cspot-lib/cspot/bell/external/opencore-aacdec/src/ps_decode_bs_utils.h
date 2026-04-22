

#ifndef PS_DECODE_BS_UTILS_H
#define PS_DECODE_BS_UTILS_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"
#include "s_bit_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void differential_Decoding(Int32 enable,
    Int32 *aIndex,
    Int32 *aPrevFrameIndex,
    Int32 DtDf,
    Int32 nrElements,
    Int32 stride,
    Int32 minIdx,
    Int32 maxIdx);

    Int32 limitMinMax(Int32 i,
                      Int32 min,
                      Int32 max);

    Int32 GetNrBitsAvailable(HANDLE_BIT_BUFFER hBitBuf);

    void map34IndexTo20(Int32 *aIndex);

#ifdef __cplusplus
}
#endif

#endif
