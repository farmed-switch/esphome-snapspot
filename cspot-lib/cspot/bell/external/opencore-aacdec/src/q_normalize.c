

#include "pv_audio_type_defs.h"
#include "s_frameinfo.h"
#include "q_normalize.h"
#include "aac_mem_funcs.h"

Int q_normalize(
    Int        qFormat[],
    const FrameInfo *pFrameInfo,
    Int32      abs_max_per_window[],
    Int32      coef[])
{
    Int    sfb;
    Int    nsfb;
    Int    win;
    Int    nwin;
    Int    sfbWidth;

    Int    shift_amt;
    Int    i;

    Int    min_q = 1000;

    Int stop_idx  = 0;

    const Int   *pSfbPerWin;
    const Int16 *pWinSfbTop;

    Int   *pQformat;
    Int32 *pCoef;

    nwin = pFrameInfo->num_win;

    pQformat   = &(qFormat[0]);
    pSfbPerWin = &(pFrameInfo->sfb_per_win[0]);

    for (win = nwin; win != 0; win--)
    {

        nsfb = *(pSfbPerWin++);

        if (nsfb < 0 || nsfb > MAXBANDS)
        {
            break;
        }

        for (sfb = nsfb; sfb != 0; sfb--)
        {
            Int qformat = *(pQformat++);
            if (qformat < min_q)
            {
                min_q = qformat;
            }
        }

    }

    pQformat   = &(qFormat[0]);
    pSfbPerWin = &(pFrameInfo->sfb_per_win[0]);
    pCoef      = &(coef[0]);

    for (win = 0; win < nwin; win++)
    {

        Int32 max = 0;
        stop_idx = 0;

        nsfb   = *(pSfbPerWin++);

        if (nsfb < 0 || nsfb > MAXBANDS)
        {
            break;
        }

        pWinSfbTop = &(pFrameInfo->win_sfb_top[win][0]);

        for (sfb = nsfb; sfb != 0; sfb--)
        {
            Int tmp1, tmp2;
            tmp1 = *(pWinSfbTop++);
            tmp2 = *(pQformat++);
            sfbWidth  = tmp1 - stop_idx;

            if (sfbWidth < 2)
            {
                break;
            }

            stop_idx += sfbWidth;

            shift_amt = tmp2 - min_q;

            if (shift_amt == 0)
            {
                Int32 tmp1, tmp2;
                tmp1 = *(pCoef++);
                tmp2 = *(pCoef++);

                for (i = (sfbWidth >> 1) - 1; i != 0; i--)
                {
                    max  |= (tmp1 >> 31) ^ tmp1;
                    max  |= (tmp2 >> 31) ^ tmp2;
                    tmp1 = *(pCoef++);
                    tmp2 = *(pCoef++);
                }
                max  |= (tmp1 >> 31) ^ tmp1;
                max  |= (tmp2 >> 31) ^ tmp2;

            }
            else
            {
                if (shift_amt < 31)
                {
                    Int32 tmp1, tmp2;
                    tmp1 = *(pCoef++) >> shift_amt;
                    tmp2 = *(pCoef--) >> shift_amt;

                    for (i = (sfbWidth >> 1) - 1; i != 0; i--)
                    {
                        *(pCoef++)   = tmp1;
                        *(pCoef++)   = tmp2;

                        max  |= (tmp1 >> 31) ^ tmp1;
                        max  |= (tmp2 >> 31) ^ tmp2;
                        tmp1 = *(pCoef++) >> shift_amt;
                        tmp2 = *(pCoef--) >> shift_amt;

                    }
                    *(pCoef++)   = tmp1;
                    *(pCoef++)   = tmp2;
                    max  |= (tmp1 >> 31) ^ tmp1;
                    max  |= (tmp2 >> 31) ^ tmp2;

                }
                else
                {
                    pv_memset(pCoef, 0, sizeof(Int32)*sfbWidth);
                    pCoef += sfbWidth;
                }
            }

            abs_max_per_window[win] = max;

        }

    }

    return min_q;

}
