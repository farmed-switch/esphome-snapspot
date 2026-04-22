

#ifndef _MP3COMMON_H
#define _MP3COMMON_H

#include "mp3dec.h"
#include "statname.h"

#define MAX_SCFBD		4
#define NGRANS_MPEG1	2
#define NGRANS_MPEG2	1

#define	SYNCWORDH		0xff
#define	SYNCWORDL		0xf0

typedef struct _MP3DecInfo {

	void *FrameHeaderPS;
	void *SideInfoPS;
	void *ScaleFactorInfoPS;
	void *HuffmanInfoPS;
	void *DequantInfoPS;
	void *IMDCTInfoPS;
	void *SubbandInfoPS;

	unsigned char mainBuf[MAINBUF_SIZE];

	int freeBitrateFlag;
	int freeBitrateSlots;

	int bitrate;
	int nChans;
	int samprate;
	int nGrans;
	int nGranSamps;
	int nSlots;
	int layer;
	MPEGVersion version;

	int mainDataBegin;
	int mainDataBytes;

	int part23Length[MAX_NGRAN][MAX_NCHAN];

} MP3DecInfo;

typedef struct _SFBandTable {
	int  l[23];
	int  s[14];
} SFBandTable;

MP3DecInfo *AllocateBuffers(void);
void FreeBuffers(MP3DecInfo *mp3DecInfo);
int CheckPadBit(MP3DecInfo *mp3DecInfo);
int UnpackFrameHeader(MP3DecInfo *mp3DecInfo, unsigned char *buf);
int UnpackSideInfo(MP3DecInfo *mp3DecInfo, unsigned char *buf);
int DecodeHuffman(MP3DecInfo *mp3DecInfo, unsigned char *buf, int *bitOffset, int huffBlockBits, int gr, int ch);
int Dequantize(MP3DecInfo *mp3DecInfo, int gr);
int IMDCT(MP3DecInfo *mp3DecInfo, int gr, int ch);
int UnpackScaleFactors(MP3DecInfo *mp3DecInfo, unsigned char *buf, int *bitOffset, int bitsAvail, int gr, int ch);
int Subband(MP3DecInfo *mp3DecInfo, short *pcmBuf);

extern const int samplerateTab[3][3];
extern const int  bitrateTab[3][3][15];
extern const int  samplesPerFrameTab[3][3];
extern const short bitsPerSlotTab[3];
extern const int  sideBytesTab[3][2];
extern const int  slotTab[3][3][15];
extern const SFBandTable sfBandTable[3][3];

#endif
