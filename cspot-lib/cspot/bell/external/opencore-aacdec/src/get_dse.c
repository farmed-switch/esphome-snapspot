

#include "pv_audio_type_defs.h"
#include "get_dse.h"
#include "ibstream.h"
#include "getbits.h"
#include "s_bits.h"

void get_dse(
    Char    *DataStreamBytes,
    BITS    *pInputStream)
{
    Int i;
    Int data_byte_align_flag;
    UInt count;
    Int esc_count;
    Char    *pDataStreamBytes;

    pDataStreamBytes = DataStreamBytes;

    get9_n_lessbits(LEN_TAG, pInputStream);

    data_byte_align_flag = get1bits(pInputStream);

    count =  get9_n_lessbits(LEN_D_CNT, pInputStream);

    if (count == (1 << LEN_D_CNT) - 1)
    {
        esc_count = (Int)get9_n_lessbits(LEN_D_ESC, pInputStream);
        count +=  esc_count;
    }

    if (data_byte_align_flag)
    {
        byte_align(pInputStream);
    }

    for (i = count; i != 0; i--)
    {
        *(pDataStreamBytes++) = (Char) get9_n_lessbits(
                                    LEN_BYTE,
                                    pInputStream);
    }

    return;

}

