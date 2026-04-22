

#include "coder.h"
#include "assembly.h"

int Dequantize(MP3DecInfo *mp3DecInfo, int gr)
{
	int i, ch, nSamps, mOut[2];
	FrameHeader *fh;
	SideInfo *si;
	ScaleFactorInfo *sfi;
	HuffmanInfo *hi;
	DequantInfo *di;
	CriticalBandInfo *cbi;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || !mp3DecInfo->SideInfoPS || !mp3DecInfo->ScaleFactorInfoPS ||
		!mp3DecInfo->HuffmanInfoPS || !mp3DecInfo->DequantInfoPS)
		return -1;

	fh = (FrameHeader *)(mp3DecInfo->FrameHeaderPS);

	si = (SideInfo *)(mp3DecInfo->SideInfoPS);
	sfi = (ScaleFactorInfo *)(mp3DecInfo->ScaleFactorInfoPS);
	hi = (HuffmanInfo *)mp3DecInfo->HuffmanInfoPS;
	di = (DequantInfo *)mp3DecInfo->DequantInfoPS;
	cbi = di->cbi;
	mOut[0] = mOut[1] = 0;

	for (ch = 0; ch < mp3DecInfo->nChans; ch++) {
		hi->gb[ch] = DequantChannel(hi->huffDecBuf[ch], di->workBuf, &hi->nonZeroBound[ch], fh,
			&si->sis[gr][ch], &sfi->sfis[gr][ch], &cbi[ch]);
	}

	if (fh->modeExt && (hi->gb[0] < 1 || hi->gb[1] < 1)) {
		for (i = 0; i < hi->nonZeroBound[0]; i++) {
			if (hi->huffDecBuf[0][i] < -0x3fffffff)	 hi->huffDecBuf[0][i] = -0x3fffffff;
			if (hi->huffDecBuf[0][i] >  0x3fffffff)	 hi->huffDecBuf[0][i] =  0x3fffffff;
		}
		for (i = 0; i < hi->nonZeroBound[1]; i++) {
			if (hi->huffDecBuf[1][i] < -0x3fffffff)	 hi->huffDecBuf[1][i] = -0x3fffffff;
			if (hi->huffDecBuf[1][i] >  0x3fffffff)	 hi->huffDecBuf[1][i] =  0x3fffffff;
		}
	}

	if (fh->modeExt >> 1) {
		if (fh->modeExt & 0x01) {

			if (cbi[1].cbType == 0)
				nSamps = fh->sfBand->l[cbi[1].cbEndL + 1];
			else
				nSamps = 3 * fh->sfBand->s[cbi[1].cbEndSMax + 1];
		} else {

			nSamps = MAX(hi->nonZeroBound[0], hi->nonZeroBound[1]);
		}
		MidSideProc(hi->huffDecBuf, nSamps, mOut);
	}

	if (fh->modeExt & 0x01) {
		nSamps = hi->nonZeroBound[0];
		if (fh->ver == MPEG1) {
			IntensityProcMPEG1(hi->huffDecBuf, nSamps, fh, &sfi->sfis[gr][1], di->cbi,
				fh->modeExt >> 1, si->sis[gr][1].mixedBlock, mOut);
		} else {
			IntensityProcMPEG2(hi->huffDecBuf, nSamps, fh, &sfi->sfis[gr][1], di->cbi, &sfi->sfjs,
				fh->modeExt >> 1, si->sis[gr][1].mixedBlock, mOut);
		}
	}

	if (fh->modeExt) {
		hi->gb[0] = CLZ(mOut[0]) - 1;
		hi->gb[1] = CLZ(mOut[1]) - 1;
		nSamps = MAX(hi->nonZeroBound[0], hi->nonZeroBound[1]);
		hi->nonZeroBound[0] = nSamps;
		hi->nonZeroBound[1] = nSamps;
	}

	return 0;
}
