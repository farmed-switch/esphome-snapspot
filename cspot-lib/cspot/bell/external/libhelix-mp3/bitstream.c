

#include "coder.h"
#include "assembly.h"

void SetBitstreamPointer(BitStreamInfo *bsi, int nBytes, unsigned char *buf)
{

	bsi->bytePtr = buf;
	bsi->iCache = 0;
	bsi->cachedBits = 0;
	bsi->nBytes = nBytes;
}

static __inline void RefillBitstreamCache(BitStreamInfo *bsi)
{
	int nBytes = bsi->nBytes;

	if (nBytes >= 4) {
		bsi->iCache  = (*bsi->bytePtr++) << 24;
		bsi->iCache |= (*bsi->bytePtr++) << 16;
		bsi->iCache |= (*bsi->bytePtr++) <<  8;
		bsi->iCache |= (*bsi->bytePtr++);
		bsi->cachedBits = 32;
		bsi->nBytes -= 4;
	} else {
		bsi->iCache = 0;
		while (nBytes--) {
			bsi->iCache |= (*bsi->bytePtr++);
			bsi->iCache <<= 8;
		}
		bsi->iCache <<= ((3 - bsi->nBytes)*8);
		bsi->cachedBits = 8*bsi->nBytes;
		bsi->nBytes = 0;
	}
}

unsigned int GetBits(BitStreamInfo *bsi, int nBits)
{
	unsigned int data, lowBits;

	nBits &= 0x1f;
	data = bsi->iCache >> (31 - nBits);
	data >>= 1;
	bsi->iCache <<= nBits;
	bsi->cachedBits -= nBits;

	if (bsi->cachedBits < 0) {
		lowBits = -bsi->cachedBits;
		RefillBitstreamCache(bsi);
		data |= bsi->iCache >> (32 - lowBits);

		bsi->cachedBits -= lowBits;
		bsi->iCache <<= lowBits;
	}

	return data;
}

int CalcBitsUsed(BitStreamInfo *bsi, unsigned char *startBuf, int startOffset)
{
	int bitsUsed;

	bitsUsed  = (bsi->bytePtr - startBuf) * 8;
	bitsUsed -= bsi->cachedBits;
	bitsUsed -= startOffset;

	return bitsUsed;
}

int CheckPadBit(MP3DecInfo *mp3DecInfo)
{
	FrameHeader *fh;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS)
		return -1;

	fh = ((FrameHeader *)(mp3DecInfo->FrameHeaderPS));

	return (fh->paddingBit ? 1 : 0);
}

int UnpackFrameHeader(MP3DecInfo *mp3DecInfo, unsigned char *buf)
{

	int verIdx;
	FrameHeader *fh;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || (buf[0] & SYNCWORDH) != SYNCWORDH || (buf[1] & SYNCWORDL) != SYNCWORDL)
		return -1;

	fh = ((FrameHeader *)(mp3DecInfo->FrameHeaderPS));

	verIdx =         (buf[1] >> 3) & 0x03;
	fh->ver =        (MPEGVersion)( verIdx == 0 ? MPEG25 : ((verIdx & 0x01) ? MPEG1 : MPEG2) );
	fh->layer = 4 - ((buf[1] >> 1) & 0x03);
	fh->crc =   1 - ((buf[1] >> 0) & 0x01);
	fh->brIdx =      (buf[2] >> 4) & 0x0f;
	fh->srIdx =      (buf[2] >> 2) & 0x03;
	fh->paddingBit = (buf[2] >> 1) & 0x01;
	fh->privateBit = (buf[2] >> 0) & 0x01;
	fh->sMode =      (StereoMode)((buf[3] >> 6) & 0x03);
	fh->modeExt =    (buf[3] >> 4) & 0x03;
	fh->copyFlag =   (buf[3] >> 3) & 0x01;
	fh->origFlag =   (buf[3] >> 2) & 0x01;
	fh->emphasis =   (buf[3] >> 0) & 0x03;

	if (fh->srIdx == 3 || fh->layer == 4 || fh->brIdx == 15)
		return -1;

	fh->sfBand = &sfBandTable[fh->ver][fh->srIdx];
	if (fh->sMode != Joint)
		fh->modeExt = 0;

	mp3DecInfo->nChans = (fh->sMode == Mono ? 1 : 2);
	mp3DecInfo->samprate = samplerateTab[fh->ver][fh->srIdx];
	mp3DecInfo->nGrans = (fh->ver == MPEG1 ? NGRANS_MPEG1 : NGRANS_MPEG2);
	mp3DecInfo->nGranSamps = ((int)samplesPerFrameTab[fh->ver][fh->layer - 1]) / mp3DecInfo->nGrans;
	mp3DecInfo->layer = fh->layer;
	mp3DecInfo->version = fh->ver;

	if (fh->brIdx) {
		mp3DecInfo->bitrate = ((int)bitrateTab[fh->ver][fh->layer - 1][fh->brIdx]) * 1000;

		mp3DecInfo->nSlots = (int)slotTab[fh->ver][fh->srIdx][fh->brIdx] -
			(int)sideBytesTab[fh->ver][(fh->sMode == Mono ? 0 : 1)] -
			4 - (fh->crc ? 2 : 0) + (fh->paddingBit ? 1 : 0);
	}

	if (fh->crc) {
		fh->CRCWord = ((int)buf[4] << 8 | (int)buf[5] << 0);
		return 6;
	} else {
		fh->CRCWord = 0;
		return 4;
	}
}

int UnpackSideInfo(MP3DecInfo *mp3DecInfo, unsigned char *buf)
{
	int gr, ch, bd, nBytes;
	BitStreamInfo bitStreamInfo, *bsi;
	FrameHeader *fh;
	SideInfo *si;
	SideInfoSub *sis;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || !mp3DecInfo->SideInfoPS)
		return -1;

	fh = ((FrameHeader *)(mp3DecInfo->FrameHeaderPS));
	si = ((SideInfo *)(mp3DecInfo->SideInfoPS));

	bsi = &bitStreamInfo;
	if (fh->ver == MPEG1) {

		nBytes = (fh->sMode == Mono ? SIBYTES_MPEG1_MONO : SIBYTES_MPEG1_STEREO);
		SetBitstreamPointer(bsi, nBytes, buf);
		si->mainDataBegin = GetBits(bsi, 9);
		si->privateBits =   GetBits(bsi, (fh->sMode == Mono ? 5 : 3));

		for (ch = 0; ch < mp3DecInfo->nChans; ch++)
			for (bd = 0; bd < MAX_SCFBD; bd++)
				si->scfsi[ch][bd] = GetBits(bsi, 1);
	} else {

		nBytes = (fh->sMode == Mono ? SIBYTES_MPEG2_MONO : SIBYTES_MPEG2_STEREO);
		SetBitstreamPointer(bsi, nBytes, buf);
		si->mainDataBegin = GetBits(bsi, 8);
		si->privateBits =   GetBits(bsi, (fh->sMode == Mono ? 1 : 2));
	}

	for(gr =0; gr < mp3DecInfo->nGrans; gr++) {
		for (ch = 0; ch < mp3DecInfo->nChans; ch++) {
			sis = &si->sis[gr][ch];

			sis->part23Length =    GetBits(bsi, 12);
			sis->nBigvals =        GetBits(bsi, 9);
			sis->globalGain =      GetBits(bsi, 8);
			sis->sfCompress =      GetBits(bsi, (fh->ver == MPEG1 ? 4 : 9));
			sis->winSwitchFlag =   GetBits(bsi, 1);

			if(sis->winSwitchFlag) {

				sis->blockType =       GetBits(bsi, 2);
				sis->mixedBlock =      GetBits(bsi, 1);
				sis->tableSelect[0] =  GetBits(bsi, 5);
				sis->tableSelect[1] =  GetBits(bsi, 5);
				sis->tableSelect[2] =  0;
				sis->subBlockGain[0] = GetBits(bsi, 3);
				sis->subBlockGain[1] = GetBits(bsi, 3);
				sis->subBlockGain[2] = GetBits(bsi, 3);

				if (sis->blockType == 0) {

					sis->nBigvals = 0;
					sis->part23Length = 0;
					sis->sfCompress = 0;
				} else if (sis->blockType == 2 && sis->mixedBlock == 0) {

					sis->region0Count = 8;
				} else {

					sis->region0Count = 7;
				}
				sis->region1Count = 20 - sis->region0Count;
			} else {

				sis->blockType = 0;
				sis->mixedBlock = 0;
				sis->tableSelect[0] =  GetBits(bsi, 5);
				sis->tableSelect[1] =  GetBits(bsi, 5);
				sis->tableSelect[2] =  GetBits(bsi, 5);
				sis->region0Count =    GetBits(bsi, 4);
				sis->region1Count =    GetBits(bsi, 3);
			}
			sis->preFlag =           (fh->ver == MPEG1 ? GetBits(bsi, 1) : 0);
			sis->sfactScale =        GetBits(bsi, 1);
			sis->count1TableSelect = GetBits(bsi, 1);
		}
	}
	mp3DecInfo->mainDataBegin = si->mainDataBegin;

	ASSERT(nBytes == CalcBitsUsed(bsi, buf, 0) >> 3);

	return nBytes;
}

