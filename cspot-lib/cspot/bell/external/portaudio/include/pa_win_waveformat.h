#ifndef PA_WIN_WAVEFORMAT_H
#define PA_WIN_WAVEFORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long PaWinWaveFormatChannelMask;

#define PAWIN_SPEAKER_FRONT_LEFT                ((PaWinWaveFormatChannelMask)0x1)
#define PAWIN_SPEAKER_FRONT_RIGHT               ((PaWinWaveFormatChannelMask)0x2)
#define PAWIN_SPEAKER_FRONT_CENTER              ((PaWinWaveFormatChannelMask)0x4)
#define PAWIN_SPEAKER_LOW_FREQUENCY             ((PaWinWaveFormatChannelMask)0x8)
#define PAWIN_SPEAKER_BACK_LEFT                 ((PaWinWaveFormatChannelMask)0x10)
#define PAWIN_SPEAKER_BACK_RIGHT                ((PaWinWaveFormatChannelMask)0x20)
#define PAWIN_SPEAKER_FRONT_LEFT_OF_CENTER      ((PaWinWaveFormatChannelMask)0x40)
#define PAWIN_SPEAKER_FRONT_RIGHT_OF_CENTER     ((PaWinWaveFormatChannelMask)0x80)
#define PAWIN_SPEAKER_BACK_CENTER               ((PaWinWaveFormatChannelMask)0x100)
#define PAWIN_SPEAKER_SIDE_LEFT                 ((PaWinWaveFormatChannelMask)0x200)
#define PAWIN_SPEAKER_SIDE_RIGHT                ((PaWinWaveFormatChannelMask)0x400)
#define PAWIN_SPEAKER_TOP_CENTER                ((PaWinWaveFormatChannelMask)0x800)
#define PAWIN_SPEAKER_TOP_FRONT_LEFT            ((PaWinWaveFormatChannelMask)0x1000)
#define PAWIN_SPEAKER_TOP_FRONT_CENTER          ((PaWinWaveFormatChannelMask)0x2000)
#define PAWIN_SPEAKER_TOP_FRONT_RIGHT           ((PaWinWaveFormatChannelMask)0x4000)
#define PAWIN_SPEAKER_TOP_BACK_LEFT             ((PaWinWaveFormatChannelMask)0x8000)
#define PAWIN_SPEAKER_TOP_BACK_CENTER           ((PaWinWaveFormatChannelMask)0x10000)
#define PAWIN_SPEAKER_TOP_BACK_RIGHT            ((PaWinWaveFormatChannelMask)0x20000)

#define PAWIN_SPEAKER_RESERVED                  ((PaWinWaveFormatChannelMask)0x7FFC0000)

#define PAWIN_SPEAKER_ALL                       ((PaWinWaveFormatChannelMask)0x80000000)

#define PAWIN_SPEAKER_DIRECTOUT                 0
#define PAWIN_SPEAKER_MONO                      (PAWIN_SPEAKER_FRONT_CENTER)
#define PAWIN_SPEAKER_STEREO                    (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT)
#define PAWIN_SPEAKER_QUAD                      (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_BACK_LEFT  | PAWIN_SPEAKER_BACK_RIGHT)
#define PAWIN_SPEAKER_SURROUND                  (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_CENTER | PAWIN_SPEAKER_BACK_CENTER)
#define PAWIN_SPEAKER_5POINT1                   (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_CENTER | PAWIN_SPEAKER_LOW_FREQUENCY | \
                                                    PAWIN_SPEAKER_BACK_LEFT  | PAWIN_SPEAKER_BACK_RIGHT)
#define PAWIN_SPEAKER_7POINT1                   (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_CENTER | PAWIN_SPEAKER_LOW_FREQUENCY | \
                                                    PAWIN_SPEAKER_BACK_LEFT | PAWIN_SPEAKER_BACK_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_LEFT_OF_CENTER | PAWIN_SPEAKER_FRONT_RIGHT_OF_CENTER)
#define PAWIN_SPEAKER_5POINT1_SURROUND          (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_CENTER | PAWIN_SPEAKER_LOW_FREQUENCY | \
                                                    PAWIN_SPEAKER_SIDE_LEFT  | PAWIN_SPEAKER_SIDE_RIGHT)
#define PAWIN_SPEAKER_7POINT1_SURROUND          (PAWIN_SPEAKER_FRONT_LEFT | PAWIN_SPEAKER_FRONT_RIGHT | \
                                                    PAWIN_SPEAKER_FRONT_CENTER | PAWIN_SPEAKER_LOW_FREQUENCY | \
                                                    PAWIN_SPEAKER_BACK_LEFT | PAWIN_SPEAKER_BACK_RIGHT | \
                                                    PAWIN_SPEAKER_SIDE_LEFT | PAWIN_SPEAKER_SIDE_RIGHT)

#define PAWIN_SPEAKER_5POINT1_BACK              PAWIN_SPEAKER_5POINT1
#define PAWIN_SPEAKER_7POINT1_WIDE              PAWIN_SPEAKER_7POINT1

#define PAWIN_SPEAKER_GROUND_FRONT_LEFT         PAWIN_SPEAKER_FRONT_LEFT
#define PAWIN_SPEAKER_GROUND_FRONT_CENTER       PAWIN_SPEAKER_FRONT_CENTER
#define PAWIN_SPEAKER_GROUND_FRONT_RIGHT        PAWIN_SPEAKER_FRONT_RIGHT
#define PAWIN_SPEAKER_GROUND_REAR_LEFT          PAWIN_SPEAKER_BACK_LEFT
#define PAWIN_SPEAKER_GROUND_REAR_RIGHT         PAWIN_SPEAKER_BACK_RIGHT
#define PAWIN_SPEAKER_TOP_MIDDLE                PAWIN_SPEAKER_TOP_CENTER
#define PAWIN_SPEAKER_SUPER_WOOFER              PAWIN_SPEAKER_LOW_FREQUENCY

#define PAWIN_SIZEOF_WAVEFORMATEX   18
#define PAWIN_SIZEOF_WAVEFORMATEXTENSIBLE (PAWIN_SIZEOF_WAVEFORMATEX + 22)

typedef struct{
    unsigned char fields[ PAWIN_SIZEOF_WAVEFORMATEXTENSIBLE ];
    unsigned long extraLongForAlignment;
} PaWinWaveFormat;

#define PAWIN_INDEXOF_WVALIDBITSPERSAMPLE       (PAWIN_SIZEOF_WAVEFORMATEX+0)
#define PAWIN_INDEXOF_DWCHANNELMASK             (PAWIN_SIZEOF_WAVEFORMATEX+2)
#define PAWIN_INDEXOF_SUBFORMAT                 (PAWIN_SIZEOF_WAVEFORMATEX+6)

#define PAWIN_WAVE_FORMAT_PCM                   (1)
#define PAWIN_WAVE_FORMAT_IEEE_FLOAT            (3)
#define PAWIN_WAVE_FORMAT_DOLBY_AC3_SPDIF       (0x0092)
#define PAWIN_WAVE_FORMAT_WMA_SPDIF             (0x0164)

int PaWin_SampleFormatToLinearWaveFormatTag( PaSampleFormat sampleFormat );

void PaWin_InitializeWaveFormatEx( PaWinWaveFormat *waveFormat,
        int numChannels, PaSampleFormat sampleFormat, int waveFormatTag, double sampleRate );

void PaWin_InitializeWaveFormatExtensible( PaWinWaveFormat *waveFormat,
        int numChannels, PaSampleFormat sampleFormat, int waveFormatTag, double sampleRate,
        PaWinWaveFormatChannelMask channelMask );

PaWinWaveFormatChannelMask PaWin_DefaultChannelMask( int numChannels );

#ifdef __cplusplus
}
#endif

#endif
