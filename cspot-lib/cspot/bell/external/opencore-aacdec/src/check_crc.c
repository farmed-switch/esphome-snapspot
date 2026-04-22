

#include "check_crc.h"

void check_crc(HANDLE_CRC hCrcBuf, UInt32 bValue, Int32 nBits)
{
    Int32 i;
    UInt32 bMask = (1UL << (nBits - 1));

    for (i = 0; i < nBits; i++, bMask >>= 1)
    {
        UInt16 flag  = (UInt16)((hCrcBuf->crcState & hCrcBuf->crcMask) ? 1 : 0);
        UInt16 flag1 = (UInt16)((bMask & bValue) ? 1 : 0);

        flag ^= flag1;
        hCrcBuf->crcState <<= 1;
        if (flag)
            hCrcBuf->crcState ^= hCrcBuf->crcPoly;
    }

}

