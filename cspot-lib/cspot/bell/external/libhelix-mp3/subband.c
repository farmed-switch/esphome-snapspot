

#include "coder.h"
#include "assembly.h"

  int Subband(MP3DecInfo *mp3DecInfo, short *pcmBuf)
{
	int b;

	IMDCTInfo *mi;
	SubbandInfo *sbi;

	if (!mp3DecInfo || !mp3DecInfo->HuffmanInfoPS || !mp3DecInfo->IMDCTInfoPS || !mp3DecInfo->SubbandInfoPS)
		return -1;

	mi = (IMDCTInfo *)(mp3DecInfo->IMDCTInfoPS);
	sbi = (SubbandInfo*)(mp3DecInfo->SubbandInfoPS);

	if (mp3DecInfo->nChans == 2) {

		for (b = 0; b < BLOCK_SIZE; b++) {
			FDCT32(mi->outBuf[0][b], sbi->vbuf + 0*32, sbi->vindex, (b & 0x01), mi->gb[0]);
			FDCT32(mi->outBuf[1][b], sbi->vbuf + 1*32, sbi->vindex, (b & 0x01), mi->gb[1]);
			PolyphaseStereo(pcmBuf, sbi->vbuf + sbi->vindex + VBUF_LENGTH * (b & 0x01), polyCoef);
			sbi->vindex = (sbi->vindex - (b & 0x01)) & 7;
			pcmBuf += (2 * NBANDS);
		}
	} else {

		for (b = 0; b < BLOCK_SIZE; b++) {
			FDCT32(mi->outBuf[0][b], sbi->vbuf + 0*32, sbi->vindex, (b & 0x01), mi->gb[0]);
			PolyphaseMono(pcmBuf, sbi->vbuf + sbi->vindex + VBUF_LENGTH * (b & 0x01), polyCoef);
			sbi->vindex = (sbi->vindex - (b & 0x01)) & 7;
			pcmBuf += NBANDS;
		}
	}

	return 0;
}

