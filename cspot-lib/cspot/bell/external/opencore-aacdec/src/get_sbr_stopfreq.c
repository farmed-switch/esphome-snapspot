

#include    "config.h"

#ifdef AAC_PLUS

#include    "get_sbr_stopfreq.h"

const UChar sbr_stopfreq_tbl[6][13] =
{

    { 21, 23, 25, 27, 29, 32, 35, 38, 41, 45, 49, 54, 59},
    { 23, 25, 27, 29, 31, 34, 37, 40, 43, 47, 51, 55, 59},
    { 32, 34, 36, 38, 40, 42, 44, 46, 49, 52, 55, 58, 61},
    { 35, 36, 38, 40, 42, 44, 46, 48, 50, 52, 55, 58, 61},
    { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 60, 62}

};

Int get_sbr_stopfreq(const Int32 fs,
                     const Int32 stop_freq)
{

    Int i;

    switch (fs)
    {
        case 48000:
            i = 0;
            break;

        case 32000:
        case 24000:
            i = 2;
            break;

        case 22050:
            i = 3;
            break;

        case 16000:
            i = 4;
            break;

        case 44100:
        default:
            i = 1;
            break;
    }

    return((Int)sbr_stopfreq_tbl[i][stop_freq]);

}

#endif
