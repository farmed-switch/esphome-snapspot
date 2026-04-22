

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_open.h"
#include    "s_sbr_header_data.h"
#include    "init_sbr_dec.h"
#include    "e_sbr_error.h"
#include    "aac_mem_funcs.h"

const SBR_HEADER_DATA defaultHeader =
{
    HEADER_NOT_INITIALIZED,
    MASTER_RESET,
    0,
    UP_BY_2,
    SBR_AMP_RES_3_0,
    5,
    0,
    0,
    SBR_FREQ_SCALE_DEFAULT,
    SBR_ALTER_SCALE_DEFAULT,
    SBR_NOISE_BANDS_DEFAULT,
    0,
    SBR_LIMITER_BANDS_DEFAULT,
    SBR_LIMITER_GAINS_DEFAULT,
    SBR_INTERPOL_FREQ_DEFAULT,
    SBR_SMOOTHING_LENGTH_DEFAULT
};

void sbr_open(Int32 sampleRate,
              SBR_DEC *sbrDec,
              SBRDECODER_DATA * self,
              Bool bDownSampledSbr)

{
    Int16 i ;

    SBR_CHANNEL *SbrChannel;

    SbrChannel = self->SbrChannel;

    for (i = 0; i < MAX_NUM_CHANNELS; i++)
    {
        pv_memset((void *)&(SbrChannel[i]),
                  0,
                  sizeof(SBR_CHANNEL));

        pv_memcpy(&(SbrChannel[i].frameData.sbr_header),
                  &defaultHeader,
                  sizeof(SBR_HEADER_DATA));

        if (sampleRate > 24000 || bDownSampledSbr)
        {
            SbrChannel[i].frameData.sbr_header.sampleRateMode = SINGLE_RATE;
        }

        SbrChannel[i].outFrameSize =
            init_sbr_dec(sampleRate,
                         self->SbrChannel[0].frameData.sbr_header.sampleRateMode,
                         sbrDec,
                         &(SbrChannel[i].frameData));

        SbrChannel[i].syncState     = UPSAMPLING;

        SbrChannel[i].frameData.sUp = 1;
    }
}

#endif

