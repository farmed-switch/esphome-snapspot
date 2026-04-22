

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO

#include "aac_mem_funcs.h"
#include "s_ps_dec.h"
#include "ps_decode_bs_utils.h"

Int32 GetNrBitsAvailable(HANDLE_BIT_BUFFER hBitBuf)
{

    return (hBitBuf->bufferLen - hBitBuf->nrBitsRead);
}

void differential_Decoding(Int32 enable,
                           Int32 *aIndex,
                           Int32 *aPrevFrameIndex,
                           Int32 DtDf,
                           Int32 nrElements,
                           Int32 stride,
                           Int32 minIdx,
                           Int32 maxIdx)
{
    Int32 i;
    Int32 *ptr_aIndex;

    if (enable == 1)
    {
        ptr_aIndex = aIndex;

        if (DtDf == 0)
        {
            *(ptr_aIndex) = limitMinMax(*ptr_aIndex, minIdx, maxIdx);
            ptr_aIndex++;

            for (i = 1; i < nrElements; i++)
            {
                *(ptr_aIndex) = limitMinMax(aIndex[i-1] + *ptr_aIndex, minIdx, maxIdx);
                ptr_aIndex++;
            }
        }
        else
        {
            if (stride == 1)
            {
                for (i = 0; i < nrElements; i++)
                {
                    *(ptr_aIndex) = limitMinMax(aPrevFrameIndex[i] + *ptr_aIndex, minIdx, maxIdx);
                    ptr_aIndex++;
                }
            }
            else
            {
                for (i = 0; i < nrElements; i++)
                {
                    *(ptr_aIndex) = limitMinMax(aPrevFrameIndex[(i<<1)] + *ptr_aIndex, minIdx, maxIdx);
                    ptr_aIndex++;
                }
            }
        }
    }
    else
    {
        pv_memset((void *)aIndex, 0, nrElements*sizeof(*aIndex));
    }
    if (stride == 2)
    {
        for (i = (nrElements << 1) - 1; i > 0; i--)
        {
            aIndex[i] = aIndex[(i>>1)];
        }
    }
}

void map34IndexTo20(Int32 *aIndex)
{

    aIndex[ 0] = ((aIndex[0] << 1) +  aIndex[1]) / 3;
    aIndex[ 1] = (aIndex[1] + (aIndex[2] << 1)) / 3;
    aIndex[ 2] = ((aIndex[3] << 1) +  aIndex[4]) / 3;
    aIndex[ 3] = (aIndex[4] + (aIndex[5] << 1)) / 3;
    aIndex[ 4] = (aIndex[ 6] +  aIndex[7]) >> 1;
    aIndex[ 5] = (aIndex[ 8] +  aIndex[9]) >> 1;
    aIndex[ 6] =   aIndex[10];
    aIndex[ 7] =   aIndex[11];
    aIndex[ 8] = (aIndex[12] +  aIndex[13]) >> 1;
    aIndex[ 9] = (aIndex[14] +  aIndex[15]) >> 1;
    aIndex[10] =   aIndex[16];
    aIndex[11] =   aIndex[17];
    aIndex[12] =   aIndex[18];
    aIndex[13] =   aIndex[19];
    aIndex[14] = (aIndex[20] +  aIndex[21]) >> 1;
    aIndex[15] = (aIndex[22] +  aIndex[23]) >> 1;
    aIndex[16] = (aIndex[24] +  aIndex[25]) >> 1;
    aIndex[17] = (aIndex[26] +  aIndex[27]) >> 1;
    aIndex[18] = (aIndex[28] +  aIndex[29] + aIndex[30] + aIndex[31]) >> 2;
    aIndex[19] = (aIndex[32] +  aIndex[33]) >> 1;
}

Int32 limitMinMax(Int32 i,
                  Int32 min,
                  Int32 max)
{
    if (i < max)
    {
        if (i > min)
        {
            return i;
        }
        else
        {
            return min;
        }
    }
    else
    {
        return max;
    }
}

#endif

#endif

