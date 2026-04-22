

#include "pv_audio_type_defs.h"
#include "s_bits.h"
#include "ibstream.h"
#include "find_adts_syncword.h"

#define FIND_ADTS_ERROR -1

Int find_adts_syncword(
    UInt32 *pSyncword,
    BITS   *pInputStream,
    Int     syncword_length,
    UInt32  syncword_mask)
{

    Int    status = SUCCESS;
    UInt   search_length;
    UInt32 adts_header = 0;
    UInt32 test_for_syncword;
    UInt32 syncword = *(pSyncword);

    if ((Int)pInputStream->usedBits <
            ((Int)pInputStream->availableBits - syncword_length))
    {
        search_length = (pInputStream->availableBits - pInputStream->usedBits);

        search_length -= syncword_length;

        adts_header  = getbits(syncword_length, pInputStream);

        test_for_syncword  = adts_header & syncword_mask;
        test_for_syncword ^= syncword;

        while ((test_for_syncword != 0) && (search_length > 0))
        {
            search_length--;

            adts_header <<= 1;
            adts_header |= getbits(1, pInputStream);

            test_for_syncword  = adts_header & syncword_mask;
            test_for_syncword ^= syncword;
        }

        if (search_length == 0)
        {
            status = FIND_ADTS_ERROR;
        }

        pInputStream->byteAlignOffset =
            (pInputStream->usedBits - syncword_length) & 0x7;

    }

    else
    {
        status = FIND_ADTS_ERROR;
    }

    *(pSyncword) = adts_header;

    return (status);

}
