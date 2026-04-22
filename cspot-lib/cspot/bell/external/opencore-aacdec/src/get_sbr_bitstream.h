

#ifndef GET_SBR_BITSTREAM_H
#define GET_SBR_BITSTREAM_H

#include "s_bits.h"
#include "ibstream.h"
#include "e_rawbitstreamconst.h"
#include "s_sbrbitstream.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void get_sbr_bitstream(SBRBITSTREAM *sbrBitStream,
    BITS *pInputStream);

#ifdef __cplusplus
}
#endif

#endif

