

#include "coder.h"
#include "assembly.h"
#include <pgmspace.h>

  static void AntiAlias(int *x, int nBfly)
{
	int k, a0, b0, c0, c1;
	const int *c;

	for (k = nBfly; k > 0; k--) {
		c = csa[0];
		x += 18;

		a0 = x[-1];			c0 = *c;	c++;	b0 = x[0];		c1 = *c;	c++;
		x[-1] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[0] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-2];			c0 = *c;	c++;	b0 = x[1];		c1 = *c;	c++;
		x[-2] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[1] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-3];			c0 = *c;	c++;	b0 = x[2];		c1 = *c;	c++;
		x[-3] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[2] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-4];			c0 = *c;	c++;	b0 = x[3];		c1 = *c;	c++;
		x[-4] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[3] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-5];			c0 = *c;	c++;	b0 = x[4];		c1 = *c;	c++;
		x[-5] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[4] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-6];			c0 = *c;	c++;	b0 = x[5];		c1 = *c;	c++;
		x[-6] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[5] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-7];			c0 = *c;	c++;	b0 = x[6];		c1 = *c;	c++;
		x[-7] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[6] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;

		a0 = x[-8];			c0 = *c;	c++;	b0 = x[7];		c1 = *c;	c++;
		x[-8] = (MULSHIFT32(c0, a0) - MULSHIFT32(c1, b0)) << 1;
		x[7] =  (MULSHIFT32(c0, b0) + MULSHIFT32(c1, a0)) << 1;
	}
}

  static void WinPrevious(int *xPrev, int *xPrevWin, int btPrev)
{
	int i, x, *xp, *xpwLo, *xpwHi, wLo, wHi;
	const int *wpLo, *wpHi;

	xp = xPrev;

	if (btPrev == 2) {

		wpLo = imdctWin[btPrev];
		xPrevWin[ 0] = MULSHIFT32(wpLo[ 6], xPrev[2]) + MULSHIFT32(wpLo[0], xPrev[6]);
		xPrevWin[ 1] = MULSHIFT32(wpLo[ 7], xPrev[1]) + MULSHIFT32(wpLo[1], xPrev[7]);
		xPrevWin[ 2] = MULSHIFT32(wpLo[ 8], xPrev[0]) + MULSHIFT32(wpLo[2], xPrev[8]);
		xPrevWin[ 3] = MULSHIFT32(wpLo[ 9], xPrev[0]) + MULSHIFT32(wpLo[3], xPrev[8]);
		xPrevWin[ 4] = MULSHIFT32(wpLo[10], xPrev[1]) + MULSHIFT32(wpLo[4], xPrev[7]);
		xPrevWin[ 5] = MULSHIFT32(wpLo[11], xPrev[2]) + MULSHIFT32(wpLo[5], xPrev[6]);
		xPrevWin[ 6] = MULSHIFT32(wpLo[ 6], xPrev[5]);
		xPrevWin[ 7] = MULSHIFT32(wpLo[ 7], xPrev[4]);
		xPrevWin[ 8] = MULSHIFT32(wpLo[ 8], xPrev[3]);
		xPrevWin[ 9] = MULSHIFT32(wpLo[ 9], xPrev[3]);
		xPrevWin[10] = MULSHIFT32(wpLo[10], xPrev[4]);
		xPrevWin[11] = MULSHIFT32(wpLo[11], xPrev[5]);
		xPrevWin[12] = xPrevWin[13] = xPrevWin[14] = xPrevWin[15] = xPrevWin[16] = xPrevWin[17] = 0;
	} else {

		wpLo = imdctWin[btPrev] + 18;
		wpHi = wpLo + 17;
		xpwLo = xPrevWin;
		xpwHi = xPrevWin + 17;
		for (i = 9; i > 0; i--) {
			x = *xp++;	wLo = *wpLo++;	wHi = *wpHi--;
			*xpwLo++ = MULSHIFT32(wLo, x);
			*xpwHi-- = MULSHIFT32(wHi, x);
		}
	}
}

  static int FreqInvertRescale(int *y, int *xPrev, int blockIdx, int es)
{
	int i, d, mOut;
	int y0, y1, y2, y3, y4, y5, y6, y7, y8;

	if (es == 0) {

		if (blockIdx & 0x01) {
			y += NBANDS;
			y0 = *y;	y += 2*NBANDS;
			y1 = *y;	y += 2*NBANDS;
			y2 = *y;	y += 2*NBANDS;
			y3 = *y;	y += 2*NBANDS;
			y4 = *y;	y += 2*NBANDS;
			y5 = *y;	y += 2*NBANDS;
			y6 = *y;	y += 2*NBANDS;
			y7 = *y;	y += 2*NBANDS;
			y8 = *y;	y += 2*NBANDS;

			y -= 18*NBANDS;
			*y = -y0;	y += 2*NBANDS;
			*y = -y1;	y += 2*NBANDS;
			*y = -y2;	y += 2*NBANDS;
			*y = -y3;	y += 2*NBANDS;
			*y = -y4;	y += 2*NBANDS;
			*y = -y5;	y += 2*NBANDS;
			*y = -y6;	y += 2*NBANDS;
			*y = -y7;	y += 2*NBANDS;
			*y = -y8;	y += 2*NBANDS;
		}
		return 0;
	} else {

		mOut = 0;
		if (blockIdx & 0x01) {

			for (i = 0; i < 18; i+=2) {
				d = *y;		CLIP_2N(d, 31 - es);	*y = d << es;	mOut |= FASTABS(*y);	y += NBANDS;
				d = -*y;	CLIP_2N(d, 31 - es);	*y = d << es;	mOut |= FASTABS(*y);	y += NBANDS;
				d = *xPrev;	CLIP_2N(d, 31 - es);	*xPrev++ = d << es;
			}
		} else {
			for (i = 0; i < 18; i+=2) {
				d = *y;		CLIP_2N(d, 31 - es);	*y = d << es;	mOut |= FASTABS(*y);	y += NBANDS;
				d = *y;		CLIP_2N(d, 31 - es);	*y = d << es;	mOut |= FASTABS(*y);	y += NBANDS;
				d = *xPrev;	CLIP_2N(d, 31 - es);	*xPrev++ = d << es;
			}
		}
		return mOut;
	}
}

static const int c9_0 = 0x6ed9eba1;
static const int c9_1 = 0x620dbe8b;
static const int c9_2 = 0x163a1a7e;
static const int c9_3 = 0x5246dd49;
static const int c9_4 = 0x7e0e2e32;

static const int c18[9] PROGMEM = {
	0x7f834ed0, 0x7ba3751d, 0x7401e4c1, 0x68d9f964, 0x5a82799a, 0x496af3e2, 0x36185aee, 0x2120fb83, 0x0b27eb5c,
};

static __inline void idct9(int *x)
{
	int a1, a2, a3, a4, a5, a6, a7, a8, a9;
	int a10, a11, a12, a13, a14, a15, a16, a17, a18;
	int a19, a20, a21, a22, a23, a24, a25, a26, a27;
	int m1, m3, m5, m6, m7, m8, m9, m10, m11, m12;
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;

	x0 = x[0]; x1 = x[1]; x2 = x[2]; x3 = x[3]; x4 = x[4];
	x5 = x[5]; x6 = x[6]; x7 = x[7]; x8 = x[8];

	a1 = x0 - x6;
	a2 = x1 - x5;
	a3 = x1 + x5;
	a4 = x2 - x4;
	a5 = x2 + x4;
	a6 = x2 + x8;
	a7 = x1 + x7;

	a8 = a6 - a5;
	a9 = a3 - a7;
	a10 = a2 - x7;
	a11 = a4 - x8;

	m1 =  MULSHIFT32(c9_0, x3);
	m3 =  MULSHIFT32(c9_0, a10);
	m5 =  MULSHIFT32(c9_1, a5);
	m6 =  MULSHIFT32(c9_2, a6);
	m7 =  MULSHIFT32(c9_1, a8);
	m8 =  MULSHIFT32(c9_2, a5);
	m9 =  MULSHIFT32(c9_3, a9);
	m10 = MULSHIFT32(c9_4, a7);
	m11 = MULSHIFT32(c9_3, a3);
	m12 = MULSHIFT32(c9_4, a9);

	a12 = x[0] +  (x[6] >> 1);
	a13 = a12  +  (  m1 << 1);
	a14 = a12  -  (  m1 << 1);
	a15 = a1   +  ( a11 >> 1);
	a16 = ( m5 << 1) + (m6 << 1);
	a17 = ( m7 << 1) - (m8 << 1);
	a18 = a16 + a17;
	a19 = ( m9 << 1) + (m10 << 1);
	a20 = (m11 << 1) - (m12 << 1);

	a21 = a20 - a19;
	a22 = a13 + a16;
	a23 = a14 + a16;
	a24 = a14 + a17;
	a25 = a13 + a17;
	a26 = a14 - a18;
	a27 = a13 - a18;

	x0 = a22 + a19;			x[0] = x0;
	x1 = a15 + (m3 << 1);	x[1] = x1;
	x2 = a24 + a20;			x[2] = x2;
	x3 = a26 - a21;			x[3] = x3;
	x4 = a1 - a11;			x[4] = x4;
	x5 = a27 + a21;			x[5] = x5;
	x6 = a25 - a20;			x[6] = x6;
	x7 = a15 - (m3 << 1);	x[7] = x7;
	x8 = a23 - a19;			x[8] = x8;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
const int fastWin36[18] PROGMEM = {
	0x42aace8b, 0xc2e92724, 0x47311c28, 0xc95f619a, 0x4a868feb, 0xd0859d8c,
	0x4c913b51, 0xd8243ea0, 0x4d413ccc, 0xe0000000, 0x4c913b51, 0xe7dbc161,
	0x4a868feb, 0xef7a6275, 0x47311c28, 0xf6a09e67, 0x42aace8b, 0xfd16d8dd,
};
#pragma GCC diagnostic pop

  static int IMDCT36(int *xCurr, int *xPrev, int *y, int btCurr, int btPrev, int blockIdx, int gb)
{
	int i, es, xBuf[18], xPrevWin[18];
	int acc1, acc2, s, d, t, mOut;
	int xo, xe, c, *xp, yLo, yHi;
	const int *cp, *wp;

	acc1 = acc2 = 0;
	xCurr += 17;

	if (gb < 7) {

		es = 7 - gb;
		for (i = 8; i >= 0; i--) {
			acc1 = ((*xCurr--) >> es) - acc1;
			acc2 = acc1 - acc2;
			acc1 = ((*xCurr--) >> es) - acc1;
			xBuf[i+9] = acc2;
			xBuf[i+0] = acc1;
			xPrev[i] >>= es;
		}
	} else {
		es = 0;

		for (i = 8; i >= 0; i--) {
			acc1 = (*xCurr--) - acc1;
			acc2 = acc1 - acc2;
			acc1 = (*xCurr--) - acc1;
			xBuf[i+9] = acc2;
			xBuf[i+0] = acc1;
		}
	}

	xBuf[9] >>= 1;
	xBuf[0] >>= 1;

	idct9(xBuf+0);
	idct9(xBuf+9);

	xp = xBuf + 8;
	cp = c18 + 8;
	mOut = 0;
	if (btPrev == 0 && btCurr == 0) {

		wp = fastWin36;
		for (i = 0; i < 9; i++) {

			c = *cp--;	xo = *(xp + 9);		xe = *xp--;

			xo = MULSHIFT32(c, xo);
			xe >>= 2;

			s = -(*xPrev);
			d = -(xe - xo);
			(*xPrev++) = xe + xo;
			t = s - d;

			yLo = (d + (MULSHIFT32(t, *wp++) << 2));
			yHi = (s + (MULSHIFT32(t, *wp++) << 2));
			y[(i)*NBANDS]    = 	yLo;
			y[(17-i)*NBANDS] =  yHi;
			mOut |= FASTABS(yLo);
			mOut |= FASTABS(yHi);
		}
	} else {

		WinPrevious(xPrev, xPrevWin, btPrev);

		wp = imdctWin[btCurr];
		for (i = 0; i < 9; i++) {
			c = *cp--;	xo = *(xp + 9);		xe = *xp--;

			xo = MULSHIFT32(c, xo);
			xe >>= 2;

			d = xe - xo;
			(*xPrev++) = xe + xo;

			yLo = (xPrevWin[i]    + MULSHIFT32(d, wp[i])) << 2;
			yHi = (xPrevWin[17-i] + MULSHIFT32(d, wp[17-i])) << 2;
			y[(i)*NBANDS]    = yLo;
			y[(17-i)*NBANDS] = yHi;
			mOut |= FASTABS(yLo);
			mOut |= FASTABS(yHi);
		}
	}

	xPrev -= 9;
	mOut |= FreqInvertRescale(y, xPrev, blockIdx, es);

	return mOut;
}

static int c3_0 = 0x6ed9eba1;
static int c6[3] = { 0x7ba3751d, 0x5a82799a, 0x2120fb83 };

static __inline void imdct12 (int *x, int *out)
{
	int a0, a1, a2;
	int x0, x1, x2, x3, x4, x5;

	x0 = *x;	x+=3;	x1 = *x;	x+=3;
	x2 = *x;	x+=3;	x3 = *x;	x+=3;
	x4 = *x;	x+=3;	x5 = *x;	x+=3;

	x4 -= x5;
	x3 -= x4;
	x2 -= x3;
	x3 -= x5;
	x1 -= x2;
	x0 -= x1;
	x1 -= x3;

	x0 >>= 1;
	x1 >>= 1;

	a0 = MULSHIFT32(c3_0, x2) << 1;
	a1 = x0 + (x4 >> 1);
	a2 = x0 - x4;
	x0 = a1 + a0;
	x2 = a2;
	x4 = a1 - a0;

	a0 = MULSHIFT32(c3_0, x3) << 1;
	a1 = x1 + (x5 >> 1);
	a2 = x1 - x5;

	x1 = MULSHIFT32(c6[0], a1 + a0) << 2;
	x3 = MULSHIFT32(c6[1], a2) << 2;
	x5 = MULSHIFT32(c6[2], a1 - a0) << 2;

	*out = x0 + x1;	out++;
	*out = x2 + x3;	out++;
	*out = x4 + x5;	out++;
	*out = x4 - x5;	out++;
	*out = x2 - x3;	out++;
	*out = x0 - x1;
}

  static int IMDCT12x3(int *xCurr, int *xPrev, int *y, int btPrev, int blockIdx, int gb)
{
	int i, es, mOut, yLo, xBuf[18], xPrevWin[18];
	const int *wp;

	es = 0;

	if (gb < 7) {
		es = 7 - gb;
		for (i = 0; i < 18; i+=2) {
			xCurr[i+0] >>= es;
			xCurr[i+1] >>= es;
			*xPrev++ >>= es;
		}
		xPrev -= 9;
	}

	imdct12(xCurr + 0, xBuf + 0);
	imdct12(xCurr + 1, xBuf + 6);
	imdct12(xCurr + 2, xBuf + 12);

	WinPrevious(xPrev, xPrevWin, btPrev);

	wp = imdctWin[2];
	mOut = 0;
	for (i = 0; i < 3; i++) {
		yLo = (xPrevWin[ 0+i] << 2);
		mOut |= FASTABS(yLo);	y[( 0+i)*NBANDS] = yLo;
		yLo = (xPrevWin[ 3+i] << 2);
		mOut |= FASTABS(yLo);	y[( 3+i)*NBANDS] = yLo;
		yLo = (xPrevWin[ 6+i] << 2) + (MULSHIFT32(wp[0+i], xBuf[3+i]));
		mOut |= FASTABS(yLo);	y[( 6+i)*NBANDS] = yLo;
		yLo = (xPrevWin[ 9+i] << 2) + (MULSHIFT32(wp[3+i], xBuf[5-i]));
		mOut |= FASTABS(yLo);	y[( 9+i)*NBANDS] = yLo;
		yLo = (xPrevWin[12+i] << 2) + (MULSHIFT32(wp[6+i], xBuf[2-i]) + MULSHIFT32(wp[0+i], xBuf[(6+3)+i]));
		mOut |= FASTABS(yLo);	y[(12+i)*NBANDS] = yLo;
		yLo = (xPrevWin[15+i] << 2) + (MULSHIFT32(wp[9+i], xBuf[0+i]) + MULSHIFT32(wp[3+i], xBuf[(6+5)-i]));
		mOut |= FASTABS(yLo);	y[(15+i)*NBANDS] = yLo;
	}

	for (i = 6; i < 9; i++)
		*xPrev++ = xBuf[i] >> 2;
	for (i = 12; i < 18; i++)
		*xPrev++ = xBuf[i] >> 2;

	xPrev -= 9;
	mOut |= FreqInvertRescale(y, xPrev, blockIdx, es);

	return mOut;
}

  static int HybridTransform(int *xCurr, int *xPrev, int y[BLOCK_SIZE][NBANDS], SideInfoSub *sis, BlockCount *bc)
{
	int xPrevWin[18], currWinIdx, prevWinIdx;
	int i, j, nBlocksOut, nonZero, mOut;
	int fiBit, xp;

	ASSERT(bc->nBlocksLong  <= NBANDS);
	ASSERT(bc->nBlocksTotal <= NBANDS);
	ASSERT(bc->nBlocksPrev  <= NBANDS);

	mOut = 0;

	for(i = 0; i < bc->nBlocksLong; i++) {

		currWinIdx = sis->blockType;
		if (sis->mixedBlock && i < bc->currWinSwitch)
			currWinIdx = 0;

		prevWinIdx = bc->prevType;
		if (i < bc->prevWinSwitch)
			 prevWinIdx = 0;

		mOut |= IMDCT36(xCurr, xPrev, &(y[0][i]), currWinIdx, prevWinIdx, i, bc->gbIn);
		xCurr += 18;
		xPrev += 9;
	}

	for (   ; i < bc->nBlocksTotal; i++) {
		ASSERT(sis->blockType == 2);

		prevWinIdx = bc->prevType;
		if (i < bc->prevWinSwitch)
			 prevWinIdx = 0;

		mOut |= IMDCT12x3(xCurr, xPrev, &(y[0][i]), prevWinIdx, i, bc->gbIn);
		xCurr += 18;
		xPrev += 9;
	}
	nBlocksOut = i;

	for (   ; i < bc->nBlocksPrev; i++) {
		prevWinIdx = bc->prevType;
		if (i < bc->prevWinSwitch)
			 prevWinIdx = 0;
		WinPrevious(xPrev, xPrevWin, prevWinIdx);

		nonZero = 0;
		fiBit = i << 31;
		for (j = 0; j < 9; j++) {
			xp = xPrevWin[2*j+0] << 2;
			nonZero |= xp;
			y[2*j+0][i] = xp;
			mOut |= FASTABS(xp);

			xp = xPrevWin[2*j+1] << 2;
			xp = (xp ^ (fiBit >> 31)) + (i & 0x01);
			nonZero |= xp;
			y[2*j+1][i] = xp;
			mOut |= FASTABS(xp);

			xPrev[j] = 0;
		}
		xPrev += 9;
		if (nonZero)
			nBlocksOut = i;
	}

	for (   ; i < 32; i++) {
		for (j = 0; j < 18; j++)
			y[j][i] = 0;
	}

	bc->gbOut = CLZ(mOut) - 1;

	return nBlocksOut;
}

  int IMDCT(MP3DecInfo *mp3DecInfo, int gr, int ch)
{
	int nBfly, blockCutoff;
	FrameHeader *fh;
	SideInfo *si;
	HuffmanInfo *hi;
	IMDCTInfo *mi;
	BlockCount bc;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || !mp3DecInfo->SideInfoPS ||
		!mp3DecInfo->HuffmanInfoPS || !mp3DecInfo->IMDCTInfoPS)
		return -1;

	fh = (FrameHeader *)(mp3DecInfo->FrameHeaderPS);
	si = (SideInfo *)(mp3DecInfo->SideInfoPS);
	hi = (HuffmanInfo*)(mp3DecInfo->HuffmanInfoPS);
	mi = (IMDCTInfo *)(mp3DecInfo->IMDCTInfoPS);

	blockCutoff = fh->sfBand->l[(fh->ver == MPEG1 ? 8 : 6)] / 18;
	if (si->sis[gr][ch].blockType != 2) {

		bc.nBlocksLong = MIN((hi->nonZeroBound[ch] + 7) / 18 + 1, 32);
		nBfly = bc.nBlocksLong - 1;
	} else if (si->sis[gr][ch].blockType == 2 && si->sis[gr][ch].mixedBlock) {

		bc.nBlocksLong = blockCutoff;
		nBfly = bc.nBlocksLong - 1;
	} else {

		bc.nBlocksLong = 0;
		nBfly = 0;
	}

	AntiAlias(hi->huffDecBuf[ch], nBfly);
	hi->nonZeroBound[ch] = MAX(hi->nonZeroBound[ch], (nBfly * 18) + 8);

	ASSERT(hi->nonZeroBound[ch] <= MAX_NSAMP);

	bc.nBlocksTotal = (hi->nonZeroBound[ch] + 17) / 18;
	bc.nBlocksPrev = mi->numPrevIMDCT[ch];
	bc.prevType = mi->prevType[ch];
	bc.prevWinSwitch = mi->prevWinSwitch[ch];
	bc.currWinSwitch = (si->sis[gr][ch].mixedBlock ? blockCutoff : 0);
	bc.gbIn = hi->gb[ch];

	mi->numPrevIMDCT[ch] = HybridTransform(hi->huffDecBuf[ch], mi->overBuf[ch], mi->outBuf[ch], &si->sis[gr][ch], &bc);
	mi->prevType[ch] = si->sis[gr][ch].blockType;
	mi->prevWinSwitch[ch] = bc.currWinSwitch;
	mi->gb[ch] = bc.gbOut;

	ASSERT(mi->numPrevIMDCT[ch] <= NBANDS);

	return 0;
}
