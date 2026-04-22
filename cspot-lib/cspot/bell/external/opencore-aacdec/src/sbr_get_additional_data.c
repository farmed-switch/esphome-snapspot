

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_additional_data.h"
#include    "buf_getbits.h"

void sbr_get_additional_data(SBR_FRAME_DATA * hFrameData,
                             BIT_BUFFER     * hBitBuf)
{
    Int32 i;

    Int32 flag = buf_getbits(hBitBuf, 1);

    if (flag)
    {
        for (i = 0; i < hFrameData->nSfb[HI]; i++)
        {
            hFrameData->addHarmonics[i] = buf_getbits(hBitBuf, 1);
        }
    }
}

#endif

