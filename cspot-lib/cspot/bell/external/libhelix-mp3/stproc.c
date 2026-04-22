

#include "coder.h"
#include "assembly.h"

void MidSideProc(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, int mOut[2])
{
	int i, xr, xl, mOutL, mOutR;

	mOutL = mOutR = 0;
	for(i = 0; i < nSamps; i++) {
		xl = x[0][i];
		xr = x[1][i];
		x[0][i] = xl + xr;
		x[1][i] = xl - xr;
		mOutL |= FASTABS(x[0][i]);
		mOutR |= FASTABS(x[1][i]);
	}
	mOut[0] |= mOutL;
	mOut[1] |= mOutR;
}

void IntensityProcMPEG1(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, FrameHeader *fh, ScaleFactorInfoSub *sfis,
						CriticalBandInfo *cbi, int midSideFlag, int mixFlag, int mOut[2])
{
	int i=0, j=0, n=0, cb=0, w=0;
	int sampsLeft, isf, mOutL, mOutR, xl, xr;
	int fl, fr, fls[3], frs[3];
	int cbStartL=0, cbStartS=0, cbEndL=0, cbEndS=0;
	int *isfTab;
	(void)mixFlag;

	if (cbi[1].cbType == 0) {

		cbStartL = cbi[1].cbEndL + 1;
		cbEndL =   cbi[0].cbEndL + 1;
		cbStartS = cbEndS = 0;
		i = fh->sfBand->l[cbStartL];
	} else if (cbi[1].cbType == 1 || cbi[1].cbType == 2) {

		cbStartS = cbi[1].cbEndSMax + 1;
		cbEndS =   cbi[0].cbEndSMax + 1;
		cbStartL = cbEndL = 0;
		i = 3 * fh->sfBand->s[cbStartS];
	}

	sampsLeft = nSamps - i;
	isfTab = (int *)ISFMpeg1[midSideFlag];
	mOutL = mOutR = 0;

	for (cb = cbStartL; cb < cbEndL && sampsLeft > 0; cb++) {
		isf = sfis->l[cb];
		if (isf == 7) {
			fl = ISFIIP[midSideFlag][0];
			fr = ISFIIP[midSideFlag][1];
		} else {
			fl = isfTab[isf];
			fr = isfTab[6] - isfTab[isf];
		}

		n = fh->sfBand->l[cb + 1] - fh->sfBand->l[cb];
		for (j = 0; j < n && sampsLeft > 0; j++, i++) {
			xr = MULSHIFT32(fr, x[0][i]) << 2;	x[1][i] = xr; mOutR |= FASTABS(xr);
			xl = MULSHIFT32(fl, x[0][i]) << 2;	x[0][i] = xl; mOutL |= FASTABS(xl);
			sampsLeft--;
		}
	}

	for (cb = cbStartS; cb < cbEndS && sampsLeft >= 3; cb++) {
		for (w = 0; w < 3; w++) {
			isf = sfis->s[cb][w];
			if (isf == 7) {
				fls[w] = ISFIIP[midSideFlag][0];
				frs[w] = ISFIIP[midSideFlag][1];
			} else {
				fls[w] = isfTab[isf];
				frs[w] = isfTab[6] - isfTab[isf];
			}
		}

		n = fh->sfBand->s[cb + 1] - fh->sfBand->s[cb];
		for (j = 0; j < n && sampsLeft >= 3; j++, i+=3) {
			xr = MULSHIFT32(frs[0], x[0][i+0]) << 2;	x[1][i+0] = xr;	mOutR |= FASTABS(xr);
			xl = MULSHIFT32(fls[0], x[0][i+0]) << 2;	x[0][i+0] = xl;	mOutL |= FASTABS(xl);
			xr = MULSHIFT32(frs[1], x[0][i+1]) << 2;	x[1][i+1] = xr;	mOutR |= FASTABS(xr);
			xl = MULSHIFT32(fls[1], x[0][i+1]) << 2;	x[0][i+1] = xl;	mOutL |= FASTABS(xl);
			xr = MULSHIFT32(frs[2], x[0][i+2]) << 2;	x[1][i+2] = xr;	mOutR |= FASTABS(xr);
			xl = MULSHIFT32(fls[2], x[0][i+2]) << 2;	x[0][i+2] = xl;	mOutL |= FASTABS(xl);
			sampsLeft -= 3;
		}
	}
	mOut[0] = mOutL;
	mOut[1] = mOutR;

	return;
}

void IntensityProcMPEG2(int x[MAX_NCHAN][MAX_NSAMP], int nSamps, FrameHeader *fh, ScaleFactorInfoSub *sfis,
						CriticalBandInfo *cbi, ScaleFactorJS *sfjs, int midSideFlag, int mixFlag, int mOut[2])
{
	int i, j, k, n, r, cb, w;
	int fl, fr, mOutL, mOutR, xl, xr;
	int sampsLeft;
	int isf, sfIdx, tmp, il[23];
	int *isfTab;
	int cbStartL, cbStartS, cbEndL, cbEndS;

	(void)mixFlag;

	isfTab = (int *)ISFMpeg2[sfjs->intensityScale][midSideFlag];
	mOutL = mOutR = 0;

	for (k = r = 0; r < 4; r++) {
		tmp = (1 << sfjs->slen[r]) - 1;
		for (j = 0; j < sfjs->nr[r]; j++, k++)
			il[k] = tmp;
	}

	if (cbi[1].cbType == 0) {

		il[21] = il[22] = 1;
		cbStartL = cbi[1].cbEndL + 1;
		cbEndL =   cbi[0].cbEndL + 1;
		i = fh->sfBand->l[cbStartL];
		sampsLeft = nSamps - i;

		for(cb = cbStartL; cb < cbEndL; cb++) {
			sfIdx = sfis->l[cb];
			if (sfIdx == il[cb]) {
				fl = ISFIIP[midSideFlag][0];
				fr = ISFIIP[midSideFlag][1];
			} else {
				isf = (sfis->l[cb] + 1) >> 1;
				fl = isfTab[(sfIdx & 0x01 ? isf : 0)];
				fr = isfTab[(sfIdx & 0x01 ? 0 : isf)];
			}
			n = MIN(fh->sfBand->l[cb + 1] - fh->sfBand->l[cb], sampsLeft);

			for(j = 0; j < n; j++, i++) {
				xr = MULSHIFT32(fr, x[0][i]) << 2;	x[1][i] = xr;	mOutR |= FASTABS(xr);
				xl = MULSHIFT32(fl, x[0][i]) << 2;	x[0][i] = xl;	mOutL |= FASTABS(xl);
			}

			sampsLeft -= n;
			if (sampsLeft == 0)
				break;
		}
	} else {

		il[12] = 1;

		for(w = 0; w < 3; w++) {
			cbStartS = cbi[1].cbEndS[w] + 1;
			cbEndS =   cbi[0].cbEndS[w] + 1;
			i = 3 * fh->sfBand->s[cbStartS] + w;

			for(cb = cbStartS; cb < cbEndS; cb++) {
				sfIdx = sfis->s[cb][w];
				if (sfIdx == il[cb]) {
					fl = ISFIIP[midSideFlag][0];
					fr = ISFIIP[midSideFlag][1];
				} else {
					isf = (sfis->s[cb][w] + 1) >> 1;
					fl = isfTab[(sfIdx & 0x01 ? isf : 0)];
					fr = isfTab[(sfIdx & 0x01 ? 0 : isf)];
				}
				n = fh->sfBand->s[cb + 1] - fh->sfBand->s[cb];

				for(j = 0; j < n; j++, i+=3) {
					xr = MULSHIFT32(fr, x[0][i]) << 2;	x[1][i] = xr;	mOutR |= FASTABS(xr);
					xl = MULSHIFT32(fl, x[0][i]) << 2;	x[0][i] = xl;	mOutL |= FASTABS(xl);
				}
			}
		}
	}
	mOut[0] = mOutL;
	mOut[1] = mOutR;

	return;
}

