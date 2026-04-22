

#ifndef E_TMP4AUDIOOBJECTTYPE_H
#define E_TMP4AUDIOOBJECTTYPE_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum eMP4AudioObjectType
    {
        MP4AUDIO_NULL            =  0,
        MP4AUDIO_AAC_MAIN        =  1,
        MP4AUDIO_AAC_LC          =  2,
        MP4AUDIO_AAC_SSR         =  3,
        MP4AUDIO_LTP             =  4,
        MP4AUDIO_SBR             =  5,
        MP4AUDIO_AAC_SCALABLE    =  6,
        MP4AUDIO_TWINVQ          =  7,
        MP4AUDIO_CELP            =  8,
        MP4AUDIO_HVXC            =  9,

        MP4AUDIO_TTSI            = 12,

        MP4AUDIO_ER_AAC_LC       = 17,

        MP4AUDIO_ER_AAC_LTP      = 19,
        MP4AUDIO_ER_AAC_SCALABLE = 20,
        MP4AUDIO_ER_TWINVQ       = 21,
        MP4AUDIO_ER_BSAC         = 22,
        MP4AUDIO_ER_AAC_LD       = 23,
        MP4AUDIO_ER_CELP         = 24,
        MP4AUDIO_ER_HVXC         = 25,
        MP4AUDIO_ER_HILN         = 26,
        MP4AUDIO_PARAMETRIC      = 27,
        MP4AUDIO_PS              = 29

    } tMP4AudioObjectType;

#ifdef __cplusplus
}
#endif

#endif

