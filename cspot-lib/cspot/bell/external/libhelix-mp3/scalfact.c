

#include "coder.h"

static const char SFLenTab[16][2] = {
	{0, 0},    {0, 1},
	{0, 2},    {0, 3},
	{3, 0},    {1, 1},
	{1, 2},    {1, 3},
	{2, 1},    {2, 2},
	{2, 3},    {3, 1},
	{3, 2},    {3, 3},
	{4, 2},    {4, 3},
};

static void UnpackSFMPEG1(BitStreamInfo *bsi, SideInfoSub *sis, ScaleFactorInfoSub *sfis, int *scfsi, int gr, ScaleFactorInfoSub *sfisGr0)
{
	int sfb;
	int slen0, slen1;

	slen0 = (int)SFLenTab[sis->sfCompress][0];
	slen1 = (int)SFLenTab[sis->sfCompress][1];

	if (sis->blockType == 2) {

		if (sis->mixedBlock) {

			for (sfb = 0; sfb < 8; sfb++)
				sfis->l[sfb] =    (char)GetBits(bsi, slen0);
			sfb = 3;
		} else {

			sfb = 0;
		}

		for (      ; sfb < 6; sfb++) {
			sfis->s[sfb][0] = (char)GetBits(bsi, slen0);
			sfis->s[sfb][1] = (char)GetBits(bsi, slen0);
			sfis->s[sfb][2] = (char)GetBits(bsi, slen0);
		}

		for (      ; sfb < 12; sfb++) {
			sfis->s[sfb][0] = (char)GetBits(bsi, slen1);
			sfis->s[sfb][1] = (char)GetBits(bsi, slen1);
			sfis->s[sfb][2] = (char)GetBits(bsi, slen1);
		}

		sfis->s[12][0] = sfis->s[12][1] = sfis->s[12][2] = 0;
	} else {

		if(gr == 0) {

			for (sfb = 0;  sfb < 11; sfb++)
				sfis->l[sfb] = (char)GetBits(bsi, slen0);
			for (sfb = 11; sfb < 21; sfb++)
				sfis->l[sfb] = (char)GetBits(bsi, slen1);
			return;
		} else {

			sfb = 0;
			if(scfsi[0])  for(  ; sfb < 6 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
			else          for(  ; sfb < 6 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen0);
			if(scfsi[1])  for(  ; sfb <11 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
			else          for(  ; sfb <11 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen0);
			if(scfsi[2])  for(  ; sfb <16 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
			else          for(  ; sfb <16 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen1);
			if(scfsi[3])  for(  ; sfb <21 ; sfb++) sfis->l[sfb] = sfisGr0->l[sfb];
			else          for(  ; sfb <21 ; sfb++) sfis->l[sfb] = (char)GetBits(bsi, slen1);
		}

		sfis->l[21] = 0;
		sfis->l[22] = 0;
	}
}

static const char NRTab[6][3][4] = {

	{	{6, 5, 5, 5},
		{3, 3, 3, 3},
		{6, 3, 3, 3},
	},
	{	{6, 5, 7, 3},
		{3, 3, 4, 2},
		{6, 3, 4, 2},
	},
	{	{11, 10, 0, 0},
		{6, 6, 0, 0},
		{6, 3, 6, 0},
	},

	{	{7, 7, 7, 0},
		{4, 4, 4, 0},
		{6, 5, 4, 0},
	},
	{	{6, 6, 6, 3},
		{4, 3, 3, 2},
		{6, 4, 3, 2},
	},
	{	{8, 8, 5, 0},
		{5, 4, 3, 0},
		{6, 6, 3, 0},
	}
};

static void UnpackSFMPEG2(BitStreamInfo *bsi, SideInfoSub *sis, ScaleFactorInfoSub *sfis, int gr, int ch, int modeExt, ScaleFactorJS *sfjs)
{

	int i, sfb, sfcIdx, btIdx, nrIdx;
	int slen[4], nr[4];
	int sfCompress, preFlag, intensityScale;
	(void)gr;

	sfCompress = sis->sfCompress;
	preFlag = 0;
	intensityScale = 0;

	if (! ((modeExt & 0x01) && (ch == 1)) ) {

		if (sfCompress < 400) {

			slen[0] = (sfCompress >> 4) / 5;
			slen[1]= (sfCompress >> 4) % 5;
			slen[2]= (sfCompress & 0x0f) >> 2;
			slen[3]= (sfCompress & 0x03);
			sfcIdx = 0;
		} else if (sfCompress < 500) {

			sfCompress -= 400;
			slen[0] = (sfCompress >> 2) / 5;
			slen[1]= (sfCompress >> 2) % 5;
			slen[2]= (sfCompress & 0x03);
			slen[3]= 0;
			sfcIdx = 1;
		} else {

			sfCompress -= 500;
			slen[0] = sfCompress / 3;
			slen[1] = sfCompress % 3;
			slen[2] = slen[3] = 0;
			if (sis->mixedBlock) {

				slen[2] = slen[1];
				slen[1] = slen[0];
			}
			preFlag = 1;
			sfcIdx = 2;
		}
	} else {

		intensityScale = sfCompress & 0x01;
		sfCompress >>= 1;
		if (sfCompress < 180) {

			slen[0] = (sfCompress / 36);
			slen[1] = (sfCompress % 36) / 6;
			slen[2] = (sfCompress % 36) % 6;
			slen[3] = 0;
			sfcIdx = 3;
		} else if (sfCompress < 244) {

			sfCompress -= 180;
			slen[0] = (sfCompress & 0x3f) >> 4;
			slen[1] = (sfCompress & 0x0f) >> 2;
			slen[2] = (sfCompress & 0x03);
			slen[3] = 0;
			sfcIdx = 4;
		} else {

			sfCompress -= 244;
			slen[0] = (sfCompress / 3);
			slen[1] = (sfCompress % 3);
			slen[2] = slen[3] = 0;
			sfcIdx = 5;
		}
	}

	btIdx = 0;
	if (sis->blockType == 2)
		btIdx = (sis->mixedBlock ? 2 : 1);
	for (i = 0; i < 4; i++)
		nr[i] = (int)NRTab[sfcIdx][btIdx][i];

	if( (modeExt & 0x01) && (ch == 1) ) {
		for (i = 0; i < 4; i++) {
			sfjs->slen[i] = slen[i];
			sfjs->nr[i] = nr[i];
		}
		sfjs->intensityScale = intensityScale;
	}
	sis->preFlag = preFlag;

	if(sis->blockType == 2) {
		if(sis->mixedBlock) {

			for (sfb=0; sfb < 6; sfb++) {
				sfis->l[sfb] = (char)GetBits(bsi, slen[0]);
			}
			sfb = 3;
			nrIdx = 1;
		} else {

			sfb = 0;
			nrIdx = 0;
		}

		for (    ; nrIdx <= 3; nrIdx++) {

			for (i=0; i < nr[nrIdx]; i++, sfb++) {
				sfis->s[sfb][0] = (char)GetBits(bsi, slen[nrIdx]);
				sfis->s[sfb][1] = (char)GetBits(bsi, slen[nrIdx]);
				sfis->s[sfb][2] = (char)GetBits(bsi, slen[nrIdx]);
			}
		}

		sfis->s[12][0] = sfis->s[12][1] = sfis->s[12][2] = 0;
	} else {

		sfb = 0;
		for (nrIdx = 0; nrIdx <= 3; nrIdx++) {

			for(i=0; i < nr[nrIdx]; i++, sfb++) {
				sfis->l[sfb] = (char)GetBits(bsi, slen[nrIdx]);
			}
		}

		sfis->l[21] = sfis->l[22] = 0;

	}
}

int UnpackScaleFactors(MP3DecInfo *mp3DecInfo, unsigned char *buf, int *bitOffset, int bitsAvail, int gr, int ch)
{
	int bitsUsed;
	unsigned char *startBuf;
	BitStreamInfo bitStreamInfo, *bsi;
	FrameHeader *fh;
	SideInfo *si;
	ScaleFactorInfo *sfi;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || !mp3DecInfo->SideInfoPS || !mp3DecInfo->ScaleFactorInfoPS)
		return -1;
	fh = ((FrameHeader *)(mp3DecInfo->FrameHeaderPS));
	si = ((SideInfo *)(mp3DecInfo->SideInfoPS));
	sfi = ((ScaleFactorInfo *)(mp3DecInfo->ScaleFactorInfoPS));

	startBuf = buf;
	bsi = &bitStreamInfo;
	SetBitstreamPointer(bsi, (bitsAvail + *bitOffset + 7) / 8, buf);
	if (*bitOffset)
		GetBits(bsi, *bitOffset);

	if (fh->ver == MPEG1)
		UnpackSFMPEG1(bsi, &si->sis[gr][ch], &sfi->sfis[gr][ch], si->scfsi[ch], gr, &sfi->sfis[0][ch]);
	else
		UnpackSFMPEG2(bsi, &si->sis[gr][ch], &sfi->sfis[gr][ch], gr, ch, fh->modeExt, &sfi->sfjs);

	mp3DecInfo->part23Length[gr][ch] = si->sis[gr][ch].part23Length;

	bitsUsed = CalcBitsUsed(bsi, buf, *bitOffset);
	buf += (bitsUsed + *bitOffset) >> 3;
	*bitOffset = (bitsUsed + *bitOffset) & 0x07;

	return (buf - startBuf);
}

