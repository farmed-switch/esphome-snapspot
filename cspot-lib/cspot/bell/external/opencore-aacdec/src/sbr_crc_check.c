

#include    "config.h"

#ifdef AAC_PLUS

#include "sbr_crc_check.h"
#include "s_crc_buffer.h"
#include "buf_getbits.h"
#include "sbr_constants.h"
#include "check_crc.h"

const unsigned short MAXCRCSTEP = 16;

Int32 sbr_crc_check(BIT_BUFFER * hBitBuf, UInt32 NrBits)
{
    Int32 crcResult = 1;
    BIT_BUFFER BitBufferCRC;
    UInt32 NrCrcBits;

    UInt32 crcCheckSum;

    Int32 i;
    CRC_BUFFER CrcBuf;
    UInt32 bValue;
    Int32 CrcStep;
    Int32 CrcNrBitsRest;

    crcCheckSum = buf_getbits(hBitBuf, SI_SBR_CRC_BITS);

    BitBufferCRC.char_ptr       = hBitBuf->char_ptr;
    BitBufferCRC.buffer_word    = hBitBuf->buffer_word;
    BitBufferCRC.buffered_bits  = hBitBuf->buffered_bits;
    BitBufferCRC.nrBitsRead     = hBitBuf->nrBitsRead;
    BitBufferCRC.bufferLen      = hBitBuf->bufferLen;

    NrCrcBits = min(NrBits, BitBufferCRC.bufferLen - BitBufferCRC.nrBitsRead);

    CrcStep = NrCrcBits / MAXCRCSTEP;
    CrcNrBitsRest = (NrCrcBits - CrcStep * MAXCRCSTEP);

    CrcBuf.crcState = CRCSTART;
    CrcBuf.crcMask  = CRCMASK;
    CrcBuf.crcPoly  = CRCPOLY;

    for (i = 0; i < CrcStep; i++)
    {
        bValue = buf_getbits(&BitBufferCRC, MAXCRCSTEP);
        check_crc(&CrcBuf, bValue, MAXCRCSTEP);
    }

    bValue = buf_getbits(&BitBufferCRC, CrcNrBitsRest);
    check_crc(&CrcBuf, bValue, CrcNrBitsRest);

    if ((UInt32)(CrcBuf.crcState & CRCRANGE) != crcCheckSum)
    {
        crcResult = 0;
    }

    return (crcResult);
}

#endif

