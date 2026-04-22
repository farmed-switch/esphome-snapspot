

#include    "get_sbr_startfreq.h"

const Int v_offset[7][16] =
{
    { -8, -7, -6, -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7},
    { -5, -4, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13},
    { -5, -3, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16},
    { -6, -4, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16},
    { -4, -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16, 20},
    { -2, -1,  0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16, 20, 24},
    { 0,  1,  2,  3,  4,  5,  6,  7,  9, 11, 13, 16, 20, 24, 28, 33}
};

Int get_sbr_startfreq(const Int32 fs,
                      const Int32 start_freq)
{
    Int k0_min = 0;
    Int32 index;

    switch (fs)
    {
        case 16000:
            index = 0;
            k0_min = 24;
            break;
        case 22050:
            index = 1;
            k0_min = 17;
            break;
        case 24000:
            index = 2;
            k0_min = 16;
            break;
        case 32000:
            index = 3;
            k0_min = 16;
            break;
        case 44100:
            index = 4;
            k0_min = 12;
            break;
        case 48000:
            index = 4;
            k0_min = 11;
            break;
        case 64000:
            index = 4;
            k0_min = 10;
            break;
        case 88200:
        case 96000:
            index = 5;
            k0_min = 7;
            break;

        default:
            index = 6;
    }
    return (k0_min + v_offset[index][start_freq]);

}

