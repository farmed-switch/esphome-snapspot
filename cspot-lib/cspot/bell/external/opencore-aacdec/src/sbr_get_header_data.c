

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_header_data.h"
#include    "sbr_constants.h"
#include    "buf_getbits.h"
#include    "aac_mem_funcs.h"

SBR_HEADER_STATUS sbr_get_header_data(SBR_HEADER_DATA   * h_sbr_header,
                                      BIT_BUFFER          * hBitBuf,
                                      SBR_SYNC_STATE     syncState)
{
    SBR_HEADER_DATA lastHeader;
    Int32 headerExtra1, headerExtra2;

    if (syncState == SBR_ACTIVE)
    {
        pv_memcpy(&lastHeader, h_sbr_header, sizeof(SBR_HEADER_DATA));
    }
    else
    {
        pv_memset((void *)&lastHeader, 0, sizeof(SBR_HEADER_DATA));
    }

    h_sbr_header->ampResolution   = buf_getbits(hBitBuf, SI_SBR_AMP_RES_BITS);
    h_sbr_header->startFreq       = buf_getbits(hBitBuf, SI_SBR_START_FREQ_BITS);
    h_sbr_header->stopFreq        = buf_getbits(hBitBuf, SI_SBR_STOP_FREQ_BITS);
    h_sbr_header->xover_band      = buf_getbits(hBitBuf, SI_SBR_XOVER_BAND_BITS);

    buf_getbits(hBitBuf, SI_SBR_RESERVED_BITS_HDR);

    headerExtra1    = buf_getbits(hBitBuf, SI_SBR_HEADER_EXTRA_1_BITS);
    headerExtra2    = buf_getbits(hBitBuf, SI_SBR_HEADER_EXTRA_2_BITS);

    if (headerExtra1)
    {
        h_sbr_header->freqScale   = buf_getbits(hBitBuf, SI_SBR_FREQ_SCALE_BITS);
        h_sbr_header->alterScale  = buf_getbits(hBitBuf, SI_SBR_ALTER_SCALE_BITS);
        h_sbr_header->noise_bands = buf_getbits(hBitBuf, SI_SBR_NOISE_BANDS_BITS);
    }
    else
    {
        h_sbr_header->freqScale   = SBR_FREQ_SCALE_DEFAULT;
        h_sbr_header->alterScale  = SBR_ALTER_SCALE_DEFAULT;
        h_sbr_header->noise_bands = SBR_NOISE_BANDS_DEFAULT;
    }

    if (headerExtra2)
    {
        h_sbr_header->limiterBands    = buf_getbits(hBitBuf, SI_SBR_LIMITER_BANDS_BITS);
        h_sbr_header->limiterGains    = buf_getbits(hBitBuf, SI_SBR_LIMITER_GAINS_BITS);
        h_sbr_header->interpolFreq    = buf_getbits(hBitBuf, SI_SBR_INTERPOL_FREQ_BITS);
        h_sbr_header->smoothingLength = buf_getbits(hBitBuf, SI_SBR_SMOOTHING_LENGTH_BITS);
    }
    else
    {
        h_sbr_header->limiterBands    = SBR_LIMITER_BANDS_DEFAULT;
        h_sbr_header->limiterGains    = SBR_LIMITER_GAINS_DEFAULT;
        h_sbr_header->interpolFreq    = SBR_INTERPOL_FREQ_DEFAULT;
        h_sbr_header->smoothingLength = SBR_SMOOTHING_LENGTH_DEFAULT;
    }

    if (syncState == SBR_ACTIVE)
    {
        h_sbr_header->status = HEADER_OK;

        if (lastHeader.startFreq   != h_sbr_header->startFreq   ||
                lastHeader.stopFreq    != h_sbr_header->stopFreq    ||
                lastHeader.xover_band  != h_sbr_header->xover_band  ||
                lastHeader.freqScale   != h_sbr_header->freqScale   ||
                lastHeader.alterScale  != h_sbr_header->alterScale  ||
                lastHeader.noise_bands != h_sbr_header->noise_bands)
        {
            h_sbr_header->status = HEADER_RESET;
        }
    }
    else
    {
        h_sbr_header->status = HEADER_RESET;
    }

    return h_sbr_header->status;
}

#endif

