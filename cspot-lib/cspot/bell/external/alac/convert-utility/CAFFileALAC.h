

#ifndef _CAFFilePortable_h
#define _CAFFilePortable_h

#if TARGET_OS_WIN32
#define ATTRIBUTE_PACKED
#pragma pack(push, 1)
#else
#define ATTRIBUTE_PACKED __attribute__((__packed__))
#endif

#include "ALACAudioTypes.h"

#define kMinCAFFPacketTableHeaderSize 24

typedef uint32_t CAFFChannelLayoutTag;

enum
{
    kCAFFChannelLayoutTag_Mono          = (100<<16) | 1,
    kCAFFChannelLayoutTag_Stereo        = (101<<16) | 2,
    kCAFFChannelLayoutTag_MPEG_3_0_B    = (113<<16) | 3,
    kCAFFChannelLayoutTag_MPEG_4_0_B    = (116<<16) | 4,
    kCAFFChannelLayoutTag_MPEG_5_0_D    = (120<<16) | 5,
    kCAFFChannelLayoutTag_MPEG_5_1_D    = (124<<16) | 6,
    kCAFFChannelLayoutTag_AAC_6_1       = (142<<16) | 7,
    kCAFFChannelLayoutTag_MPEG_7_1_B	= (127<<16) | 8
};

static const CAFFChannelLayoutTag	CAFFChannelLayoutTags[kALACMaxChannels] =
{
    kCAFFChannelLayoutTag_Mono,
    kCAFFChannelLayoutTag_Stereo,
    kCAFFChannelLayoutTag_MPEG_3_0_B,
    kCAFFChannelLayoutTag_MPEG_4_0_B,
    kCAFFChannelLayoutTag_MPEG_5_0_D,
    kCAFFChannelLayoutTag_MPEG_5_1_D,
    kCAFFChannelLayoutTag_AAC_6_1,
    kCAFFChannelLayoutTag_MPEG_7_1_B
};

enum {
	k_port__port_CAF_FileType				= 'caff',
	k_port_CAF_FileVersion_Initial	= 1
};

enum {
	k_port_CAF_StreamDescriptionChunkID = 'desc',
	k_port_CAF_AudioDataChunkID			= 'data',
	k_port_CAF_ChannelLayoutChunkID		= 'chan',
	k_port_CAF_MagicCookieID			= 'kuki',
	k_port_CAF_PacketTableChunkID		= 'pakt',
	k_port_CAF_FreeTableChunkID			= 'free'
};

struct port_CAFFileHeader
{
    uint32_t          mFileType;
    uint16_t			mFileVersion;
    uint16_t			mFileFlags;
} ATTRIBUTE_PACKED;
typedef struct CAFFileHeader CAFFileHeader;

struct port_CAFChunkHeader
{
    uint32_t          mChunkType;
    int64_t          mChunkSize;

} ATTRIBUTE_PACKED;

typedef struct port_CAFChunkHeader port_CAFChunkHeader;

struct port_CAFAudioDescription
{
    double mSampleRate;
    uint32_t  mFormatID;
    uint32_t  mFormatFlags;
    uint32_t  mBytesPerPacket;
    uint32_t  mFramesPerPacket;
    uint32_t  mChannelsPerFrame;
    uint32_t  mBitsPerChannel;
} ATTRIBUTE_PACKED;
typedef struct port_CAFAudioDescription  port_CAFAudioDescription;

enum
{
    k_port_CAFLinearPCMFormatFlagIsFloat				= (1L << 0),
    k_port_CAFLinearPCMFormatFlagIsLittleEndian		= (1L << 1)
};

struct port_CAFPacketTableHeader
{
    int64_t  mNumberPackets;
    int64_t  mNumberValidFrames;
    int32_t  mPrimingFrames;
    int32_t  mRemainderFrames;

    uint8_t   mPacketDescriptions[1];
} ATTRIBUTE_PACKED;
typedef struct port_CAFPacketTableHeader port_CAFPacketTableHeader;

struct port_CAFDataChunk
{
    uint32_t mEditCount;
    uint8_t mData[1];
} ATTRIBUTE_PACKED;
typedef struct port_CAFDataChunk port_CAFDataChunk;

int32_t FindCAFFPacketTableStart(FILE * inputFile, int32_t * paktPos, int32_t * paktSize);
void WriteCAFFcaffChunk(FILE * outputFile);
void WriteCAFFdescChunk(FILE * outputFile, AudioFormatDescription theOutputFormat);
void WriteCAFFdataChunk(FILE * outputFile);
void WriteCAFFkukiChunk(FILE * outputFile, void * inCookie, uint32_t inCookieSize);
void WriteCAFFChunkSize(FILE * outputFile, int64_t numDataBytes);
void WriteCAFFchanChunk(FILE * outputFile, uint32_t inChannelTag);
void WriteCAFFfreeChunk(FILE * outputFile, uint32_t theSize);
void WriteCAFFpaktChunkHeader(FILE * outputFile, port_CAFPacketTableHeader * thePacketTableHeader, uint32_t thePacketTableSize);
void GetBERInteger(int32_t theOriginalValue, uint8_t * theBuffer, int32_t * theBERSize);
uint32_t ReadBERInteger(uint8_t * theInputBuffer, int32_t * ioNumBytes);
int32_t BuildBasePacketTable(AudioFormatDescription theInputFormat, int32_t inputDataSize, int32_t * thePacketTableSize, port_CAFPacketTableHeader * thePacketTableHeader);
uint32_t GetMagicCookieSizeFromCAFFkuki(FILE * inputFile);
int32_t GetMagicCookieFromCAFFkuki(FILE * inputFile, uint8_t * outMagicCookie, uint32_t * ioMagicCookieSize);
bool FindCAFFDataStart(FILE * inputFile, int32_t * dataPos, int32_t * dataSize);
bool GetCAFFdescFormat(FILE * inputFile, AudioFormatDescription * theInputFormat);

#if TARGET_OS_WIN32
#pragma pack(pop)
#endif

#endif
