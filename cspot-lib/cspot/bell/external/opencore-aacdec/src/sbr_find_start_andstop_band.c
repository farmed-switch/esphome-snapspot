

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_find_start_andstop_band.h"
#include    "get_sbr_startfreq.h"
#include    "get_sbr_stopfreq.h"

SBR_ERROR sbr_find_start_andstop_band(const Int32 samplingFreq,
                                      const Int32 startFreq,
                                      const Int32 stopFreq,
                                      Int   *lsbM,
                                      Int   *usb)
{

    *lsbM = get_sbr_startfreq(samplingFreq, startFreq);

    if (*lsbM == 0)
    {
        return(SBRDEC_ILLEGAL_SCFACTORS);
    }

    if (stopFreq < 13)
    {
        *usb = get_sbr_stopfreq(samplingFreq, stopFreq);
    }
    else if (stopFreq == 13)
    {
        *usb = 64;
    }
    else if (stopFreq == 14)
    {
        *usb = (*lsbM) << 1;
    }
    else
    {
        *usb = 3 * *lsbM;
    }

    if (*usb > 64)
    {
        *usb = 64;
    }

    if ((*usb - *lsbM) > 48)
    {

        return(SBRDEC_INVALID_BITSTREAM);
    }

    if ((samplingFreq == 44100) && ((*usb - *lsbM) > 35))
    {

        return(SBRDEC_INVALID_BITSTREAM);
    }

    if ((samplingFreq >= 48000) && ((*usb - *lsbM) > 32))
    {

        return(SBRDEC_INVALID_BITSTREAM);
    }

    return(SBRDEC_OK);

}

#endif

