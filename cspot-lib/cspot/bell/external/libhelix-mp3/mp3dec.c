

#include "string.h"

#include "mp3common.h"

#ifdef PROFILE
#include "systime.h"
#endif

HMP3Decoder MP3InitDecoder(void)
{
	MP3DecInfo *mp3DecInfo;

	mp3DecInfo = AllocateBuffers();

	return (HMP3Decoder)mp3DecInfo;
}

void MP3FreeDecoder(HMP3Decoder hMP3Decoder)
{
	MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;

	if (!mp3DecInfo)
		return;

	FreeBuffers(mp3DecInfo);
}

int MP3FindSyncWord(unsigned char *buf, int nBytes)
{
	int i;

	for (i = 0; i < nBytes - 1; i++) {
		if ( (buf[i+0] & SYNCWORDH) == SYNCWORDH && (buf[i+1] & SYNCWORDL) == SYNCWORDL )
			return i;
	}

	return -1;
}

static int MP3FindFreeSync(unsigned char *buf, unsigned char firstFH[4], int nBytes)
{
	int offset = 0;
	unsigned char *bufPtr = buf;

	while (1) {
		offset = MP3FindSyncWord(bufPtr, nBytes);
		bufPtr += offset;
		if (offset < 0) {
			return -1;
		} else if ( (bufPtr[0] == firstFH[0]) && (bufPtr[1] == firstFH[1]) && ((bufPtr[2] & 0xfc) == (firstFH[2] & 0xfc)) ) {

			if ((firstFH[2] >> 1) & 0x01)
				bufPtr--;
			return bufPtr - buf;
		}
		bufPtr += 3;
		nBytes -= (offset + 3);
	};

	return -1;
}

void MP3GetLastFrameInfo(HMP3Decoder hMP3Decoder, MP3FrameInfo *mp3FrameInfo)
{
	MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;

	if (!mp3DecInfo || mp3DecInfo->layer != 3) {
		mp3FrameInfo->bitrate = 0;
		mp3FrameInfo->nChans = 0;
		mp3FrameInfo->samprate = 0;
		mp3FrameInfo->bitsPerSample = 0;
		mp3FrameInfo->outputSamps = 0;
		mp3FrameInfo->layer = 0;
		mp3FrameInfo->version = 0;
	} else {
		mp3FrameInfo->bitrate = mp3DecInfo->bitrate;
		mp3FrameInfo->nChans = mp3DecInfo->nChans;
		mp3FrameInfo->samprate = mp3DecInfo->samprate;
		mp3FrameInfo->bitsPerSample = 16;
		mp3FrameInfo->outputSamps = mp3DecInfo->nChans * (int)samplesPerFrameTab[mp3DecInfo->version][mp3DecInfo->layer - 1];
		mp3FrameInfo->layer = mp3DecInfo->layer;
		mp3FrameInfo->version = mp3DecInfo->version;
	}
}

int MP3GetNextFrameInfo(HMP3Decoder hMP3Decoder, MP3FrameInfo *mp3FrameInfo, unsigned char *buf)
{
	MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;

	if (!mp3DecInfo)
		return ERR_MP3_NULL_POINTER;

	if (UnpackFrameHeader(mp3DecInfo, buf) == -1 || mp3DecInfo->layer != 3)
		return ERR_MP3_INVALID_FRAMEHEADER;

	MP3GetLastFrameInfo(mp3DecInfo, mp3FrameInfo);

	return ERR_MP3_NONE;
}

static void MP3ClearBadFrame(MP3DecInfo *mp3DecInfo, short *outbuf)
{
	int i;

	if (!mp3DecInfo)
		return;

	for (i = 0; i < mp3DecInfo->nGrans * mp3DecInfo->nGranSamps * mp3DecInfo->nChans; i++)
		outbuf[i] = 0;
}

int MP3Decode(HMP3Decoder hMP3Decoder, unsigned char **inbuf, int *bytesLeft, short *outbuf, int useSize)
{
	int offset, bitOffset, mainBits, gr, ch, fhBytes, siBytes, freeFrameBytes;
	int prevBitOffset, sfBlockBits, huffBlockBits;
	unsigned char *mainPtr;
	MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;

	#ifdef PROFILE
	long time;
	#endif

	if (!mp3DecInfo)
		return ERR_MP3_NULL_POINTER;

	fhBytes = UnpackFrameHeader(mp3DecInfo, *inbuf);
	if (fhBytes < 0)
		return ERR_MP3_INVALID_FRAMEHEADER;
	*inbuf += fhBytes;

#ifdef PROFILE
	time = systime_get();
#endif

	siBytes = UnpackSideInfo(mp3DecInfo, *inbuf);
	if (siBytes < 0) {
		MP3ClearBadFrame(mp3DecInfo, outbuf);
		return ERR_MP3_INVALID_SIDEINFO;
	}
	*inbuf += siBytes;
	*bytesLeft -= (fhBytes + siBytes);
#ifdef PROFILE
	time = systime_get() - time;
	printf("UnpackSideInfo: %i ms\n", time);
#endif

	if (mp3DecInfo->bitrate == 0 || mp3DecInfo->freeBitrateFlag) {
		if (!mp3DecInfo->freeBitrateFlag) {

			mp3DecInfo->freeBitrateFlag = 1;
			mp3DecInfo->freeBitrateSlots = MP3FindFreeSync(*inbuf, *inbuf - fhBytes - siBytes, *bytesLeft);
			if (mp3DecInfo->freeBitrateSlots < 0) {
				MP3ClearBadFrame(mp3DecInfo, outbuf);
				return ERR_MP3_FREE_BITRATE_SYNC;
			}
			freeFrameBytes = mp3DecInfo->freeBitrateSlots + fhBytes + siBytes;
			mp3DecInfo->bitrate = (freeFrameBytes * mp3DecInfo->samprate * 8) / (mp3DecInfo->nGrans * mp3DecInfo->nGranSamps);
		}
		mp3DecInfo->nSlots = mp3DecInfo->freeBitrateSlots + CheckPadBit(mp3DecInfo);
	}

	if (useSize) {
		mp3DecInfo->nSlots = *bytesLeft;
		if (mp3DecInfo->mainDataBegin != 0 || mp3DecInfo->nSlots <= 0) {

			MP3ClearBadFrame(mp3DecInfo, outbuf);
			return ERR_MP3_INVALID_FRAMEHEADER;
		}

		mp3DecInfo->mainDataBytes = mp3DecInfo->nSlots;
		mainPtr = *inbuf;
		*inbuf += mp3DecInfo->nSlots;
		*bytesLeft -= (mp3DecInfo->nSlots);
	} else {

		if (mp3DecInfo->nSlots > *bytesLeft) {
			MP3ClearBadFrame(mp3DecInfo, outbuf);
			return ERR_MP3_INDATA_UNDERFLOW;
		}

#ifdef PROFILE
	time = systime_get();
#endif

		if (mp3DecInfo->mainDataBytes >= mp3DecInfo->mainDataBegin) {

			memmove(mp3DecInfo->mainBuf, mp3DecInfo->mainBuf + mp3DecInfo->mainDataBytes - mp3DecInfo->mainDataBegin, mp3DecInfo->mainDataBegin);
			memcpy(mp3DecInfo->mainBuf + mp3DecInfo->mainDataBegin, *inbuf, mp3DecInfo->nSlots);

			mp3DecInfo->mainDataBytes = mp3DecInfo->mainDataBegin + mp3DecInfo->nSlots;
			*inbuf += mp3DecInfo->nSlots;
			*bytesLeft -= (mp3DecInfo->nSlots);
			mainPtr = mp3DecInfo->mainBuf;
		} else {

			memcpy(mp3DecInfo->mainBuf + mp3DecInfo->mainDataBytes, *inbuf, mp3DecInfo->nSlots);
			mp3DecInfo->mainDataBytes += mp3DecInfo->nSlots;
			*inbuf += mp3DecInfo->nSlots;
			*bytesLeft -= (mp3DecInfo->nSlots);
			MP3ClearBadFrame(mp3DecInfo, outbuf);
			return ERR_MP3_MAINDATA_UNDERFLOW;
		}
#ifdef PROFILE
	time = systime_get() - time;
	printf("data buffer filling: %i ms\n", time);
#endif

	}
	bitOffset = 0;
	mainBits = mp3DecInfo->mainDataBytes * 8;

	for (gr = 0; gr < mp3DecInfo->nGrans; gr++) {
		for (ch = 0; ch < mp3DecInfo->nChans; ch++) {

			#ifdef PROFILE
				time = systime_get();
			#endif

			prevBitOffset = bitOffset;
			offset = UnpackScaleFactors(mp3DecInfo, mainPtr, &bitOffset, mainBits, gr, ch);
			#ifdef PROFILE
				time = systime_get() - time;
				printf("UnpackScaleFactors: %i ms\n", time);
			#endif

			sfBlockBits = 8*offset - prevBitOffset + bitOffset;
			huffBlockBits = mp3DecInfo->part23Length[gr][ch] - sfBlockBits;
			mainPtr += offset;
			mainBits -= sfBlockBits;

			if (offset < 0 || mainBits < huffBlockBits) {
				MP3ClearBadFrame(mp3DecInfo, outbuf);
				return ERR_MP3_INVALID_SCALEFACT;
			}

			#ifdef PROFILE
				time = systime_get();
			#endif

			prevBitOffset = bitOffset;
			offset = DecodeHuffman(mp3DecInfo, mainPtr, &bitOffset, huffBlockBits, gr, ch);
			if (offset < 0) {
				MP3ClearBadFrame(mp3DecInfo, outbuf);
				return ERR_MP3_INVALID_HUFFCODES;
			}
			#ifdef PROFILE
				time = systime_get() - time;
				printf("Huffman: %i ms\n", time);
			#endif

			mainPtr += offset;
			mainBits -= (8*offset - prevBitOffset + bitOffset);
		}

		#ifdef PROFILE
			time = systime_get();
		#endif

		if (Dequantize(mp3DecInfo, gr) < 0) {
			MP3ClearBadFrame(mp3DecInfo, outbuf);
			return ERR_MP3_INVALID_DEQUANTIZE;
		}
		#ifdef PROFILE
			time = systime_get() - time;
			printf("Dequantize: %i ms\n", time);
		#endif

		for (ch = 0; ch < mp3DecInfo->nChans; ch++)
		{
		#ifdef PROFILE
			time = systime_get();
		#endif
			if (IMDCT(mp3DecInfo, gr, ch) < 0) {
				MP3ClearBadFrame(mp3DecInfo, outbuf);
				return ERR_MP3_INVALID_IMDCT;
			}
		#ifdef PROFILE
			time = systime_get() - time;
			printf("IMDCT: %i ms\n", time);
		#endif
		}

		#ifdef PROFILE
			time = systime_get();
		#endif

		if (Subband(mp3DecInfo, outbuf + gr*mp3DecInfo->nGranSamps*mp3DecInfo->nChans) < 0) {
			MP3ClearBadFrame(mp3DecInfo, outbuf);
			return ERR_MP3_INVALID_SUBBAND;
		}
		#ifdef PROFILE
			time = systime_get() - time;
			printf("Subband: %i ms\n", time);
		#endif

	}
	return ERR_MP3_NONE;
}
