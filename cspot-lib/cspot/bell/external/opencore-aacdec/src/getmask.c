

#include    "pv_audio_type_defs.h"
#include    "huffman.h"
#include    "aac_mem_funcs.h"
#include    "e_maskstatus.h"

Int getmask(
    FrameInfo   *pFrameInfo,
    BITS        *pInputStream,
    Int         group[],
    Int         max_sfb,
    Int         mask[])
{

    Int     win;
    Int     sfb;
    Int     mask_present;
    Int    *pMask;
    Int    *pGroup;
    Int     nwin;
    Int     nCall;
    Int     nToDo;
    UInt32  tempMask;
    UInt32  bitmask;

    pMask  = mask;
    pGroup = group;

    mask_present =
        get9_n_lessbits(
            LEN_MASK_PRES,
            pInputStream);

    switch (mask_present)
    {
        case(MASK_NOT_PRESENT):

            break;

        case(MASK_ALL_FRAME):

            nwin = pFrameInfo->num_win;
            for (win = 0; win < nwin; win = *(pGroup++))
            {
                for (sfb = pFrameInfo->sfb_per_win[win]; sfb > 0; sfb--)
                {
                    *(pMask++) = 1;
                }

            }

            break;

        case(MASK_FROM_BITSTREAM):

            nwin = pFrameInfo->num_win;
            for (win = 0; win < nwin; win = *(pGroup++))
            {

                nToDo = max_sfb;

                while (nToDo > 0)
                {
                    nCall = nToDo;

                    if (nCall > MAX_GETBITS)
                    {
                        nCall = MAX_GETBITS;
                    }

                    tempMask =
                        getbits(
                            nCall,
                            pInputStream);

                    bitmask = (UInt32) 1 << (nCall - 1);
                    for (sfb = nCall; sfb > 0; sfb--)
                    {
                        *(pMask++) = (Int)((tempMask & bitmask) >> (sfb - 1));
                        bitmask >>= 1;
                    }

                    nToDo -= nCall;
                }

                nCall = pFrameInfo->sfb_per_win[win] - max_sfb;

                if (nCall >= 0)
                {
                    pv_memset(pMask,
                              0,
                              nCall*sizeof(*pMask));

                    pMask += nCall;
                }
                else
                {
                    mask_present = MASK_ERROR;
                    break;
                }

            }

            break;

        default:

            break;

    }

    return mask_present;

}
