

#ifndef UNPACK_IDX_H
#define UNPACK_IDX_H

#include    "pv_audio_type_defs.h"
#include    "s_hcb.h"
#include    "s_bits.h"

#define DIMENSION_4     4

#ifdef __cplusplus
extern "C"
{
#endif

    void unpack_idx(
        Int16  QuantSpec[],
        Int  codeword_indx,
        const Hcb *pHuffCodebook,
        BITS  *pInputStream,
        Int *max);
    void unpack_idx_sgn(
        Int16  quant_spec[],
        Int  codeword_indx,
        const Hcb *pHuffCodebook,
        BITS  *pInputStream,
        Int *max);
    void unpack_idx_esc(
        Int16  quant_spec[],
        Int  codeword_indx,
        const Hcb *pHuffCodebook,
        BITS  *pInputStream,
        Int *max);

#ifdef __cplusplus
}
#endif

#endif

