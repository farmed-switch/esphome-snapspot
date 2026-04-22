

#ifndef ALACAUDIOTYPES_H
#define ALACAUDIOTYPES_H

#if PRAGMA_ONCE
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
    #pragma pack(2)
#endif

#include <stdint.h>

#if defined(__ppc__)
#define TARGET_RT_BIG_ENDIAN 1
#elif defined(__ppc64__)
#define TARGET_RT_BIG_ENDIAN 1
#endif

#define kChannelAtomSize 12

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"

enum
{
    kALAC_UnimplementedError   = -4,
    kALAC_FileNotFoundError    = -43,
    kALAC_ParamError           = -50,
    kALAC_MemFullError         = -108
};

enum
{
    kALACFormatAppleLossless = 'alac',
    kALACFormatLinearPCM = 'lpcm'
};

enum
{
    kALACMaxChannels	= 8,
    kALACMaxEscapeHeaderBytes = 8,
    kALACMaxSearches	= 16,
    kALACMaxCoefs		= 16,
    kALACDefaultFramesPerPacket = 4096
};

typedef uint32_t ALACChannelLayoutTag;

enum
{
    kALACFormatFlagIsFloat                     = (1 << 0),
    kALACFormatFlagIsBigEndian                 = (1 << 1),
    kALACFormatFlagIsSignedInteger             = (1 << 2),
    kALACFormatFlagIsPacked                    = (1 << 3),
    kALACFormatFlagIsAlignedHigh               = (1 << 4),
};

enum
{
#if TARGET_RT_BIG_ENDIAN
    kALACFormatFlagsNativeEndian       = kALACFormatFlagIsBigEndian
#else
    kALACFormatFlagsNativeEndian       = 0
#endif
};

typedef double alac_float64_t;

enum
{
    kALACChannelLayoutTag_Mono          = (100<<16) | 1,
    kALACChannelLayoutTag_Stereo        = (101<<16) | 2,
    kALACChannelLayoutTag_MPEG_3_0_B    = (113<<16) | 3,
    kALACChannelLayoutTag_MPEG_4_0_B    = (116<<16) | 4,
    kALACChannelLayoutTag_MPEG_5_0_D    = (120<<16) | 5,
    kALACChannelLayoutTag_MPEG_5_1_D    = (124<<16) | 6,
    kALACChannelLayoutTag_AAC_6_1       = (142<<16) | 7,
    kALACChannelLayoutTag_MPEG_7_1_B	= (127<<16) | 8
};

static const ALACChannelLayoutTag	ALACChannelLayoutTags[kALACMaxChannels] =
{
    kALACChannelLayoutTag_Mono,
    kALACChannelLayoutTag_Stereo,
    kALACChannelLayoutTag_MPEG_3_0_B,
    kALACChannelLayoutTag_MPEG_4_0_B,
    kALACChannelLayoutTag_MPEG_5_0_D,
    kALACChannelLayoutTag_MPEG_5_1_D,
    kALACChannelLayoutTag_AAC_6_1,
    kALACChannelLayoutTag_MPEG_7_1_B
};

struct ALACAudioChannelLayout
{
    ALACChannelLayoutTag          mChannelLayoutTag;
    uint32_t                      mChannelBitmap;
    uint32_t                      mNumberChannelDescriptions;
};
typedef struct ALACAudioChannelLayout ALACAudioChannelLayout;

struct AudioFormatDescription
{
    alac_float64_t mSampleRate;
    uint32_t  mFormatID;
    uint32_t  mFormatFlags;
    uint32_t  mBytesPerPacket;
    uint32_t  mFramesPerPacket;
    uint32_t  mBytesPerFrame;
    uint32_t  mChannelsPerFrame;
    uint32_t  mBitsPerChannel;
    uint32_t  mReserved;
};
typedef struct AudioFormatDescription  AudioFormatDescription;

enum
{
	kALACCodecFormat		= 'alac',
	kALACVersion			= 0,
	kALACCompatibleVersion	= kALACVersion,
	kALACDefaultFrameSize	= 4096
};

typedef struct ALACSpecificConfig
{
	uint32_t				frameLength;
	uint8_t					compatibleVersion;
	uint8_t					bitDepth;
	uint8_t					pb;
	uint8_t					mb;
	uint8_t					kb;
	uint8_t					numChannels;
	uint16_t				maxRun;
	uint32_t				maxFrameBytes;
	uint32_t				avgBitRate;
	uint32_t				sampleRate;

} ALACSpecificConfig;

enum
{
	AudioChannelLayoutAID = 'chan'
};

#pragma GCC diagnostic pop

#if PRAGMA_STRUCT_ALIGN
    #pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
    #pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
    #pragma pack()
#endif

#ifdef __cplusplus
}
#endif

#endif
