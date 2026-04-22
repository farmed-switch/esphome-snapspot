

#ifndef _CODER_H
#define _CODER_H

#pragma GCC optimize ("O3")

#include "mp3common.h"

#if defined(ASSERT)
#undef ASSERT
#endif
#if defined(_WIN32) && defined(_M_IX86) && (defined (_DEBUG) || defined (REL_ENABLE_ASSERTS))
#define ASSERT(x) if (!(x)) __asm int 3;
#else
#define ASSERT(x)
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define CLIP_2N(y, n) { \
	int sign = (y) >> 31;  \
	if (sign != (y) >> (n))  { \
		(y) = sign ^ ((1 << (n)) - 1); \
	} \
}

#define SIBYTES_MPEG1_MONO		17
#define SIBYTES_MPEG1_STEREO	32
#define SIBYTES_MPEG2_MONO		 9
#define SIBYTES_MPEG2_STEREO	17

#define POW43_FRACBITS_LOW		22
#define POW43_FRACBITS_HIGH		12

#define DQ_FRACBITS_OUT			25
#define	IMDCT_SCALE				2

#define	HUFF_PAIRTABS			32
#define BLOCK_SIZE				18
#define	NBANDS					32
#define MAX_REORDER_SAMPS		((192-126)*3)
#define VBUF_LENGTH				(17 * 2 * NBANDS)

#define	SetBitstreamPointer	STATNAME(SetBitstreamPointer)
#define	GetBits				STATNAME(GetBits)
#define	CalcBitsUsed		STATNAME(CalcBitsUsed)
#define	DequantChannel		STATNAME(DequantChannel)
#define	MidSideProc			STATNAME(MidSideProc)
#define	IntensityProcMPEG1	STATNAME(IntensityProcMPEG1)
#define	IntensityProcMPEG2	STATNAME(IntensityProcMPEG2)
#define PolyphaseMono		STATNAME(PolyphaseMono)
#define PolyphaseStereo		STATNAME(PolyphaseStereo)
#define FDCT32				STATNAME(FDCT32)

#define	ISFMpeg1			STATNAME(ISFMpeg1)
#define	ISFMpeg2			STATNAME(ISFMpeg2)
#define	ISFIIP				STATNAME(ISFIIP)
#define uniqueIDTab			STATNAME(uniqueIDTab)
#define	coef32				STATNAME(coef32)
#define	polyCoef			STATNAME(polyCoef)
#define	csa					STATNAME(csa)
#define	imdctWin			STATNAME(imdctWin)

#define	huffTable			STATNAME(huffTable)
#define	huffTabOffset		STATNAME(huffTabOffset)
#define	huffTabLookup		STATNAME(huffTabLookup)
#define	quadTable			STATNAME(quadTable)
#define	quadTabOffset		STATNAME(quadTabOffset)
#define	quadTabMaxBits		STATNAME(quadTabMaxBits)

typedef enum {
	Stereo = 0x00,
	Joint = 0x01,
	Dual = 0x02,
	Mono = 0x03
} StereoMode;

typedef struct _BitStreamInfo {
	unsigned char *bytePtr;
	unsigned int iCache;
	int cachedBits;
	int nBytes;
} BitStreamInfo;

typedef struct _FrameHeader {
    MPEGVersion ver;
    int layer;
    int crc;
    int brIdx;
    int srIdx;
    int paddingBit;
    int privateBit;
    StereoMode sMode;
    int modeExt;
    int copyFlag;
    int origFlag;
    int emphasis;
    int CRCWord;

	const SFBandTable *sfBand;
} FrameHeader;

typedef struct _SideInfoSub {
    int part23Length;
    int nBigvals;
    int globalGain;
    int sfCompress;
    int winSwitchFlag;
    int blockType;
    int mixedBlock;
    int tableSelect[3];
    int subBlockGain[3];
    int region0Count;
    int region1Count;
    int preFlag;
    int sfactScale;
    int count1TableSelect;
} SideInfoSub;

typedef struct _SideInfo {
	int mainDataBegin;
	int privateBits;
	int scfsi[MAX_NCHAN][MAX_SCFBD];

	SideInfoSub	sis[MAX_NGRAN][MAX_NCHAN];
} SideInfo;

typedef struct {
    int cbType;
    int cbEndS[3];
	int cbEndSMax;
    int cbEndL;
} CriticalBandInfo;

typedef struct _DequantInfo {
	int workBuf[MAX_REORDER_SAMPS];
	CriticalBandInfo cbi[MAX_NCHAN];
} DequantInfo;

typedef struct _HuffmanInfo {
	int huffDecBuf[MAX_NCHAN][MAX_NSAMP];
	int nonZeroBound[MAX_NCHAN];
	int gb[MAX_NCHAN];
} HuffmanInfo;

typedef enum _HuffTabType {
	noBits,
	oneShot,
	loopNoLinbits,
	loopLinbits,
	quadA,
	quadB,
	invalidTab
} HuffTabType;

typedef struct _HuffTabLookup {
	int	linBits;
	int   tabType;
} HuffTabLookup;

typedef struct _IMDCTInfo {
	int outBuf[MAX_NCHAN][BLOCK_SIZE][NBANDS];
	int overBuf[MAX_NCHAN][MAX_NSAMP / 2];
	int numPrevIMDCT[MAX_NCHAN];
	int prevType[MAX_NCHAN];
	int prevWinSwitch[MAX_NCHAN];
	int gb[MAX_NCHAN];
} IMDCTInfo;

typedef struct _BlockCount {
	int nBlocksLong;
	int nBlocksTotal;
	int nBlocksPrev;
	int prevType;
	int prevWinSwitch;
	int currWinSwitch;
	int gbIn;
	int gbOut;
} BlockCount;

typedef struct _ScaleFactorInfoSub {
	char l[23];
	char s[13][3];
} ScaleFactorInfoSub;

typedef struct _ScaleFactorJS {
	int intensityScale;
	int	slen[4];
	int	nr[4];
} ScaleFactorJS;

typedef struct _ScaleFactorInfo {
	ScaleFactorInfoSub sfis[MAX_NGRAN][MAX_NCHAN];
	ScaleFactorJS sfjs;
} ScaleFactorInfo;

typedef struct _SubbandInfo {
	int vbuf[MAX_NCHAN * VBUF_LENGTH];
	int vindex;
} SubbandInfo;

void SetBitstreamPointer(BitStreamInfo *bsi, int nBytes, unsigned char *buf);
unsigned int GetBits(BitStreamInfo *bsi, int nBits);
int CalcBitsUsed(BitStreamInfo *bsi, unsigned char *startBuf, int startOffset);

int DequantChannel(int *sampleBuf, int *workBuf, int *nonZeroBound, FrameHeader *fh, SideInfoSub *sis,
					ScaleFactorInfoSub *sfis, CriticalBandInfo *cbi);
void MidSideProc(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, int mOut[2]);
void IntensityProcMPEG1(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, FrameHeader *fh, ScaleFactorInfoSub *sfis,
						CriticalBandInfo *cbi, int midSideFlag, int mixFlag, int mOut[2]);
void IntensityProcMPEG2(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, FrameHeader *fh, ScaleFactorInfoSub *sfis,
						CriticalBandInfo *cbi, ScaleFactorJS *sfjs, int midSideFlag, int mixFlag, int mOut[2]);

void FDCT32(int *x, int *d, int offset, int oddBlock, int gb);

extern const HuffTabLookup huffTabLookup[HUFF_PAIRTABS];
extern const int huffTabOffset[HUFF_PAIRTABS];
extern const unsigned short huffTable[];
extern const unsigned char quadTable[64+16];
extern const int quadTabOffset[2];
extern const int quadTabMaxBits[2];

#ifdef __cplusplus
extern "C" {
#endif
void PolyphaseMono(short *pcm, int *vbuf, const int *coefBase);
void PolyphaseStereo(short *pcm, int *vbuf, const int *coefBase);
#ifdef __cplusplus
}
#endif

extern const int imdctWin[4][36];
extern const int ISFMpeg1[2][7];
extern const int ISFMpeg2[2][2][16];
extern const int ISFIIP[2][2];
extern const int csa[8][2];
extern const int coef32[31];
extern const int polyCoef[264];

#endif
