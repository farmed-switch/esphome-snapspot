

#include <stdint.h>
#define Word64 uint64_t
 #ifdef ESP8266
#  include "pgmspace.h"
#elif defined(ESP_PLATFORM) && __has_include(<pgm_space.h>)
#  include <pgm_space.h>
#else
#  define PROGMEM
#  define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#  define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef _MP3DEC_H
#define _MP3DEC_H

#if defined(_WIN32) && !defined(_WIN32_WCE)
#
#elif defined(_WIN32) && defined(WINCE_EMULATOR)
#
#elif defined(ARM_ADS)
#
#elif defined(__APPLE__)
#
#elif defined(_SYMBIAN) && defined(__WINS__)
#
#elif defined(__GNUC__) && defined(__thumb__)
#
#elif defined(__GNUC__) && defined(__i386__)
#
#elif defined(__amd64__)
#
#elif defined(_OPENWAVE_SIMULATOR) || defined(_OPENWAVE_ARMULATOR)
#
#elif defined (ESP_PLATFORM)
#
#else
#error No platform defined. See valid options in mp3dec.h
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAINBUF_SIZE	1940

#define MAX_NGRAN		2
#define MAX_NCHAN		2
#define MAX_NSAMP		576

typedef enum {
	MPEG1 =  0,
	MPEG2 =  1,
	MPEG25 = 2
} MPEGVersion;

typedef void *HMP3Decoder;

enum {
	ERR_MP3_NONE =                  0,
	ERR_MP3_INDATA_UNDERFLOW =     -1,
	ERR_MP3_MAINDATA_UNDERFLOW =   -2,
	ERR_MP3_FREE_BITRATE_SYNC =    -3,
	ERR_MP3_OUT_OF_MEMORY =	       -4,
	ERR_MP3_NULL_POINTER =         -5,
	ERR_MP3_INVALID_FRAMEHEADER =  -6,
	ERR_MP3_INVALID_SIDEINFO =     -7,
	ERR_MP3_INVALID_SCALEFACT =    -8,
	ERR_MP3_INVALID_HUFFCODES =    -9,
	ERR_MP3_INVALID_DEQUANTIZE =   -10,
	ERR_MP3_INVALID_IMDCT =        -11,
	ERR_MP3_INVALID_SUBBAND =      -12,

	ERR_UNKNOWN =                  -9999
};

typedef struct _MP3FrameInfo {
	int bitrate;
	int nChans;
	int samprate;
	int bitsPerSample;
	int outputSamps;
	int layer;
	int version;
} MP3FrameInfo;

HMP3Decoder MP3InitDecoder(void);
void MP3FreeDecoder(HMP3Decoder hMP3Decoder);
int MP3Decode(HMP3Decoder hMP3Decoder, unsigned char **inbuf, int *bytesLeft, short *outbuf, int useSize);

void MP3GetLastFrameInfo(HMP3Decoder hMP3Decoder, MP3FrameInfo *mp3FrameInfo);
int MP3GetNextFrameInfo(HMP3Decoder hMP3Decoder, MP3FrameInfo *mp3FrameInfo, unsigned char *buf);
int MP3FindSyncWord(unsigned char *buf, int nBytes);

#ifdef __cplusplus
}
#endif

#endif
