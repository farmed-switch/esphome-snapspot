#ifndef PA_WIN_DS_H
#define PA_WIN_DS_H

#include "portaudio.h"
#include "pa_win_waveformat.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define paWinDirectSoundUseLowLevelLatencyParameters            (0x01)
#define paWinDirectSoundUseChannelMask                          (0x04)

typedef struct PaWinDirectSoundStreamInfo{
    unsigned long size;
    PaHostApiTypeId hostApiType;
    unsigned long version;

    unsigned long flags;

    unsigned long framesPerBuffer;

    PaWinWaveFormatChannelMask channelMask;

}PaWinDirectSoundStreamInfo;

#ifdef __cplusplus
}
#endif

#endif
