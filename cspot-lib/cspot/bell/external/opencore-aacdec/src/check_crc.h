

#ifndef CHECK_CRC_H
#define CHECK_CRC_H

#include "pv_audio_type_defs.h"
#include "s_crc_buffer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void check_crc(HANDLE_CRC hCrcBuf,
    UInt32 bValue,
    Int32 nBits);

#ifdef __cplusplus
}
#endif

#endif

