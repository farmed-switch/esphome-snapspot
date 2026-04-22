

#include "config.h"
#include "pv_audio_type_defs.h"
#include "pv_normalize.h"

#if defined(PV_ARM_V5)
#elif defined(PV_ARM_GCC_V5)

#else

Int pv_normalize(Int32 x)
{

    Int i;

    if (x > 0x0FFFFFFF)
    {
        i = 0;
    }
    else if (x > 0x00FFFFFF)
    {
        i = 3;
    }
    else if (x > 0x0000FFFF)
    {
        i  = x > 0x000FFFFF ?  7 :  11;
    }
    else
    {
        if (x > 0x000000FF)
        {
            i  = x > 0x00000FFF ?  15 :  19;
        }
        else
        {
            i  = x > 0x0000000F ?  23 :  27;
        }
    }

    x <<= i;

    switch (x & 0x78000000)
    {
        case 0x08000000:
            i += 3;
            break;

        case 0x18000000:
        case 0x10000000:
            i += 2;
            break;
        case 0x28000000:
        case 0x20000000:
        case 0x38000000:
        case 0x30000000:
            i++;

        default:
            ;
    }

    return i;

}

#endif

