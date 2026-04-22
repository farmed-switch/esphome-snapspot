

#ifndef SBR_CRC_CHECK_H
#define SBR_CRC_CHECK_H

#include "pv_audio_type_defs.h"
#include "s_bit_buffer.h"

#define CRCPOLY  0x0233
#define CRCMASK  0x0200
#define CRCSTART 0x0000
#define CRCRANGE 0x03FF

#define SBR_EXTENSION      13
#define SBR_EXTENSION_CRC  14

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

Int32 sbr_crc_check(BIT_BUFFER * hBitBuf,
                    UInt32 NrBits);

#endif

