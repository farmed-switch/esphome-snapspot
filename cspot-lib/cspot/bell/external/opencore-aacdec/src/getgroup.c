

#include    "pv_audio_type_defs.h"
#include    "huffman.h"

#define     SEVEN   7

void getgroup(
    Int         group[],
    BITS        *pInputStream)
{
    Int      win;
    Int     *pGroup;
    UInt     mask;
    UInt     groupBits;

    pGroup      = group;

    mask        = 0x40;

    groupBits =
        get9_n_lessbits(
            SEVEN,
            pInputStream);

    for (win = 1; win < NUM_SHORT_WINDOWS; win++)
    {
        if ((groupBits & mask) == 0)
        {
            *pGroup++ = win;

        }

        mask >>= 1;

    }

    *pGroup = win;

}
