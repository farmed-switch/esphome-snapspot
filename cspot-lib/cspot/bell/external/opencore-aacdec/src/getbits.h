

#ifndef GETBITS_H
#define GETBITS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "ibstream.h"

#define INBUF_ARRAY_INDEX_SHIFT  (3)
#define INBUF_BIT_WIDTH         (1<<(INBUF_ARRAY_INDEX_SHIFT))
#define INBUF_BIT_MODULO_MASK   ((INBUF_BIT_WIDTH)-1)

#define MAX_GETBITS             (25)

#define  CHECK_INPUT_BUFFER_LIMITS  1

    static inline UInt32 getbits(
        const UInt  neededBits,
        BITS       *pInputStream)
    {
        UInt32   returnValue = 0;
        UInt     offset;
        UInt     bitIndex;
        UChar    *pElem;

        offset = (pInputStream->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

        pElem = pInputStream->pBuffer + offset;

#if CHECK_INPUT_BUFFER_LIMITS

        offset =  pInputStream->inputBufferCurrentLength - offset;

        if (offset > 3)
        {
            returnValue = (((UInt32) * (pElem)) << 24) |
                          (((UInt32) * (pElem + 1)) << 16) |
                          (((UInt32) * (pElem + 2)) << 8) |
                          ((UInt32) * (pElem + 3));
        }
        else
        {

            switch (offset)
            {
                case 3:
                    returnValue  = (((UInt32) * (pElem + 2)) << 8);
                case 2:
                    returnValue |= (((UInt32) * (pElem + 1)) << 16);
                case 1:
                    returnValue |= (((UInt32) * (pElem)) << 24);
                default:
                    break;
            }
        }

#else

        returnValue = (((UInt32) * (pElem)) << 24) |
                      (((UInt32) * (pElem + 1)) << 16) |
                      (((UInt32) * (pElem + 2)) << 8) |
                      ((UInt32) * (pElem + 3));
#endif

        bitIndex = (UInt)((pInputStream->usedBits) & INBUF_BIT_MODULO_MASK);

        returnValue = returnValue << (bitIndex);

        returnValue = returnValue >> (32 - neededBits);

        pInputStream->usedBits += neededBits;

        return (returnValue);

    }

    static inline UInt get1bits(
        BITS       *pInputStream)
    {
        UInt     returnValue;
        UInt     offset;
        UInt     bitIndex;
        UChar    *pElem;

        offset = (pInputStream->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

        pElem = pInputStream->pBuffer + offset;

#if CHECK_INPUT_BUFFER_LIMITS
        returnValue = (offset < pInputStream->inputBufferCurrentLength) ? ((UInt) * (pElem)) : 0;
#else
        returnValue = ((UInt32) * (pElem));
#endif

        bitIndex = (UInt)((pInputStream->usedBits++) & INBUF_BIT_MODULO_MASK);

        returnValue = 0xFF & (returnValue << (bitIndex));

        return ((UInt)(returnValue >> 7));

    }

    static inline UInt get9_n_lessbits(
        const UInt  neededBits,
        BITS       *pInputStream)

    {
        UInt     returnValue;
        UInt     offset;
        UInt     bitIndex;
        UChar    *pElem;

        offset = (pInputStream->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

        pElem = pInputStream->pBuffer + offset;

#if CHECK_INPUT_BUFFER_LIMITS

        offset =  pInputStream->inputBufferCurrentLength - offset;

        if (offset > 1)
        {
            returnValue = (((UInt32) * (pElem)) << 8) |
                          ((UInt32) * (pElem + 1));
        }
        else
        {

            switch (offset)
            {
                case 1:
                    returnValue  = (((UInt32) * (pElem)) << 8);
                    break;
                default:
                    returnValue = 0;
                    break;
            }
        }

#else
        returnValue = (((UInt32) * (pElem)) << 8) |
                      ((UInt32) * (pElem + 1)) ;
#endif

        bitIndex = (UInt)((pInputStream->usedBits) & INBUF_BIT_MODULO_MASK);

        pInputStream->usedBits += neededBits;

        returnValue = 0xFFFF & (returnValue << (bitIndex));

        return (UInt)(returnValue >> (16 - neededBits));

    }

    static inline UInt32 get17_n_lessbits(
        const UInt  neededBits,
        BITS       *pInputStream)
    {
        UInt32   returnValue;
        UInt     offset;
        UInt     bitIndex;
        UChar    *pElem;

        offset = (pInputStream->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

        pElem = pInputStream->pBuffer + offset;

#if CHECK_INPUT_BUFFER_LIMITS

        offset =  pInputStream->inputBufferCurrentLength - offset;

        if (offset > 2)
        {
            returnValue = (((UInt32) * (pElem)) << 16) |
                          (((UInt32) * (pElem + 1)) << 8) |
                          ((UInt32)  * (pElem + 2));
        }
        else
        {

            returnValue = 0;
            switch (offset)
            {
                case 2:
                    returnValue  = (((UInt32) * (pElem + 1)) << 8);
                case 1:
                    returnValue |= (((UInt32) * (pElem)) << 16);
                default:
                    break;
            }
        }

#else

        returnValue = (((UInt32) * (pElem)) << 16) |
                      (((UInt32) * (pElem + 1)) << 8) |
                      ((UInt32)  * (pElem + 2));
#endif

        bitIndex = (UInt)((pInputStream->usedBits) & INBUF_BIT_MODULO_MASK);

        returnValue = 0xFFFFFF & (returnValue << (bitIndex));

        returnValue = returnValue >> (24 - neededBits);

        pInputStream->usedBits += neededBits;

        return (returnValue);

    }

#ifdef __cplusplus
}
#endif

#endif

