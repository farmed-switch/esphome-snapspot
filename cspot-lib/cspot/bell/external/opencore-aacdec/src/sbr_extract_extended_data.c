

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_extract_extended_data.h"
#include    "buf_getbits.h"

#ifdef PARAMETRICSTEREO
#include    "ps_read_data.h"
#endif

void sbr_extract_extended_data(BIT_BUFFER * hBitBuf
#ifdef PARAMETRICSTEREO
                               , HANDLE_PS_DEC hParametricStereoDec
#endif
                              )
{
    Int32 extended_data;
    Int32 i;
    Int32 nBitsLeft;
    Int32 extension_id;

    extended_data = buf_get_1bit(hBitBuf);

    if (extended_data)
    {
        Int32 cnt;

        cnt = buf_getbits(hBitBuf, SI_SBR_EXTENSION_SIZE_BITS);
        if (cnt == (1 << SI_SBR_EXTENSION_SIZE_BITS) - 1)
        {
            cnt += buf_getbits(hBitBuf, SI_SBR_EXTENSION_ESC_COUNT_BITS);
        }

        nBitsLeft = (cnt << 3);
        while (nBitsLeft > 7)
        {
            extension_id = buf_getbits(hBitBuf, SI_SBR_EXTENSION_ID_BITS);
            nBitsLeft -= SI_SBR_EXTENSION_ID_BITS;

            switch (extension_id)
            {
#ifdef HQ_SBR
#ifdef PARAMETRICSTEREO

                case EXTENSION_ID_PS_CODING:

                    if (hParametricStereoDec != NULL)
                    {
                        if (!hParametricStereoDec->psDetected)
                        {

                            hParametricStereoDec->psDetected = 1;
                        }

                        nBitsLeft -= ps_read_data(hParametricStereoDec,
                                                  hBitBuf,
                                                  nBitsLeft);

                    }

                    break;
#endif
#endif
                case 0:

                default:

                    cnt = nBitsLeft >> 3;

                    for (i = 0; i < cnt; i++)
                    {
                        buf_getbits(hBitBuf, 8);
                    }

                    nBitsLeft -= (cnt << 3);
            }
        }

        buf_getbits(hBitBuf, nBitsLeft);
    }
}

#endif

