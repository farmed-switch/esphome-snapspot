

#include "coder.h"
#include "assembly.h"

typedef int ARRAY3[3];

static const char preTab[22] = { 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0 };

const int pow14[4] PROGMEM = {
	0x7fffffff, 0x6ba27e65, 0x5a82799a, 0x4c1bf829
};

const int pow43_14[4][16] PROGMEM = {
{	0x00000000, 0x10000000, 0x285145f3, 0x453a5cdb,
	0x0cb2ff53, 0x111989d6, 0x15ce31c8, 0x1ac7f203,
	0x20000000, 0x257106b9, 0x2b16b4a3, 0x30ed74b4,
	0x36f23fa5, 0x3d227bd3, 0x437be656, 0x49fc823c, },

{	0x00000000, 0x0d744fcd, 0x21e71f26, 0x3a36abd9,
	0x0aadc084, 0x0e610e6e, 0x12560c1d, 0x168523cf,
	0x1ae89f99, 0x1f7c03a4, 0x243bae49, 0x29249c67,
	0x2e34420f, 0x33686f85, 0x38bf3dff, 0x3e370182, },

{	0x00000000, 0x0b504f33, 0x1c823e07, 0x30f39a55,
	0x08facd62, 0x0c176319, 0x0f6b3522, 0x12efe2ad,
	0x16a09e66, 0x1a79a317, 0x1e77e301, 0x2298d5b4,
	0x26da56fc, 0x2b3a902a, 0x2fb7e7e7, 0x3450f650, },

{	0x00000000, 0x09837f05, 0x17f910d7, 0x2929c7a9,
	0x078d0dfa, 0x0a2ae661, 0x0cf73154, 0x0fec91cb,
	0x1306fe0a, 0x16434a6c, 0x199ee595, 0x1d17ae3d,
	0x20abd76a, 0x2459d551, 0x28204fbb, 0x2bfe1808, },
};

const int pow43[] PROGMEM = {
	0x1428a2fa, 0x15db1bd6, 0x1796302c, 0x19598d85,
	0x1b24e8bb, 0x1cf7fcfa, 0x1ed28af2, 0x20b4582a,
	0x229d2e6e, 0x248cdb55, 0x26832fda, 0x28800000,
	0x2a832287, 0x2c8c70a8, 0x2e9bc5d8, 0x30b0ff99,
	0x32cbfd4a, 0x34eca001, 0x3712ca62, 0x393e6088,
	0x3b6f47e0, 0x3da56717, 0x3fe0a5fc, 0x4220ed72,
	0x44662758, 0x46b03e7c, 0x48ff1e87, 0x4b52b3f3,
	0x4daaebfd, 0x5007b497, 0x5268fc62, 0x54ceb29c,
	0x5738c721, 0x59a72a59, 0x5c19cd35, 0x5e90a129,
	0x610b9821, 0x638aa47f, 0x660db90f, 0x6894c90b,
	0x6b1fc80c, 0x6daeaa0d, 0x70416360, 0x72d7e8b0,
	0x75722ef9, 0x78102b85, 0x7ab1d3ec, 0x7d571e09,
};

#define SQRTHALF 0x5a82799a

static const unsigned int poly43lo[5] PROGMEM = { 0x29a0bda9, 0xb02e4828, 0x5957aa1b, 0x236c498d, 0xff581859 };
static const unsigned int poly43hi[5] PROGMEM = { 0x10852163, 0xd333f6a4, 0x46e9408b, 0x27c2cef0, 0xfef577b4 };

const int pow2exp[8] PROGMEM = { 14, 13, 11, 10, 9, 7, 6, 5 };

const int pow2frac[8] PROGMEM = {
	0x6597fa94, 0x50a28be6, 0x7fffffff, 0x6597fa94,
	0x50a28be6, 0x7fffffff, 0x6597fa94, 0x50a28be6
};

  static int DequantBlock(int *inbuf, int *outbuf, int num, int scale)
{
	int tab4[4];
	int scalef, scalei, shift;
	int sx, x, y;
	int mask = 0;
	const int *tab16;
	const unsigned int *coef;

	tab16 = pow43_14[scale & 0x3];
	scalef = pow14[scale & 0x3];
	scalei = MIN(scale >> 2, 31);

	shift = MIN(scalei + 3, 31);
	shift = MAX(shift, 0);
	tab4[0] = 0;
	tab4[1] = tab16[1] >> shift;
	tab4[2] = tab16[2] >> shift;
	tab4[3] = tab16[3] >> shift;

	do {

		sx = *inbuf++;
		x = sx & 0x7fffffff;

		if (x < 4) {

			y = tab4[x];

		} else if (x < 16) {

			y = tab16[x];
			y = (scalei < 0) ? y << -scalei : y >> scalei;

		} else {

			if (x < 64) {

				y = pow43[x-16];

				y = MULSHIFT32(y, scalef);
				shift = scalei - 3;

			} else {

				x <<= 17;
				shift = 0;
				if (x < 0x08000000)
					x <<= 4, shift += 4;
				if (x < 0x20000000)
					x <<= 2, shift += 2;
				if (x < 0x40000000)
					x <<= 1, shift += 1;

				coef = (x < SQRTHALF) ? poly43lo : poly43hi;

				y = coef[0];
				y = MULSHIFT32(y, x) + coef[1];
				y = MULSHIFT32(y, x) + coef[2];
				y = MULSHIFT32(y, x) + coef[3];
				y = MULSHIFT32(y, x) + coef[4];
				y = MULSHIFT32(y, pow2frac[shift]) << 3;

				y = MULSHIFT32(y, scalef);
				shift = scalei - pow2exp[shift];
			}

			if (shift < 0) {
				shift = -shift;
				if (y > (0x7fffffff >> shift))
					y = 0x7fffffff;
				else
					y <<= shift;
			} else {
				y >>= shift;
			}
		}

		mask |= y;
		*outbuf++ = (sx < 0) ? -y : y;

	} while (--num);

	return mask;
}

  int DequantChannel(int *sampleBuf, int *workBuf, int *nonZeroBound, FrameHeader *fh, SideInfoSub *sis,
					ScaleFactorInfoSub *sfis, CriticalBandInfo *cbi)
{
	int i, j, w, cb;
	int   cbEndL, cbStartS, cbEndS;
	int nSamps, nonZero, sfactMultiplier, gbMask;
	int globalGain, gainI;
	int cbMax[3];
	ARRAY3 *buf;

	if (sis->blockType == 2) {

		if (sis->mixedBlock) {
			cbEndL = (fh->ver == MPEG1 ? 8 : 6);
			cbStartS = 3;
		} else {
			cbEndL = 0;
			cbStartS = 0;
		}
		cbEndS = 13;
	} else {

		cbEndL =   22;
		cbStartS = 13;
		cbEndS =   13;
	}
	cbMax[2] = cbMax[1] = cbMax[0] = 0;
	gbMask = 0;
	i = 0;

	sfactMultiplier = 2 * (sis->sfactScale + 1);

	globalGain = sis->globalGain;
	if (fh->modeExt >> 1)
		 globalGain -= 2;
	globalGain += IMDCT_SCALE;

	for (cb = 0; cb < cbEndL; cb++) {

		nonZero = 0;
		nSamps = fh->sfBand->l[cb + 1] - fh->sfBand->l[cb];
		gainI = 210 - globalGain + sfactMultiplier * (sfis->l[cb] + (sis->preFlag ? (int)preTab[cb] : 0));

		nonZero |= DequantBlock(sampleBuf + i, sampleBuf + i, nSamps, gainI);
		i += nSamps;

		if (nonZero)
			cbMax[0] = cb;
		gbMask |= nonZero;

		if (i >= *nonZeroBound)
			break;
	}

	cbi->cbType = 0;
	cbi->cbEndL  = cbMax[0];
	cbi->cbEndS[0] = cbi->cbEndS[1] = cbi->cbEndS[2] = 0;
	cbi->cbEndSMax = 0;

	if (cbStartS >= 12)
		return CLZ(gbMask) - 1;

	cbMax[2] = cbMax[1] = cbMax[0] = cbStartS;
	for (cb = cbStartS; cb < cbEndS; cb++) {

		nSamps = fh->sfBand->s[cb + 1] - fh->sfBand->s[cb];
		for (w = 0; w < 3; w++) {
			nonZero =  0;
			gainI = 210 - globalGain + 8*sis->subBlockGain[w] + sfactMultiplier*(sfis->s[cb][w]);

			nonZero |= DequantBlock(sampleBuf + i + nSamps*w, workBuf + nSamps*w, nSamps, gainI);

			if (nonZero)
				cbMax[w] = cb;
			gbMask |= nonZero;
		}

		buf = (ARRAY3 *)(sampleBuf + i);
		i += 3*nSamps;
		for (j = 0; j < nSamps; j++) {
			buf[j][0] = workBuf[0*nSamps + j];
			buf[j][1] = workBuf[1*nSamps + j];
			buf[j][2] = workBuf[2*nSamps + j];
		}

		ASSERT(3*nSamps <= MAX_REORDER_SAMPS);

		if (i >= *nonZeroBound)
			break;
	}

	*nonZeroBound = i;

	ASSERT(*nonZeroBound <= MAX_NSAMP);

	cbi->cbType = (sis->mixedBlock ? 2 : 1);

	cbi->cbEndS[0] = cbMax[0];
	cbi->cbEndS[1] = cbMax[1];
	cbi->cbEndS[2] = cbMax[2];

	cbi->cbEndSMax = cbMax[0];
	cbi->cbEndSMax = MAX(cbi->cbEndSMax, cbMax[1]);
	cbi->cbEndSMax = MAX(cbi->cbEndSMax, cbMax[2]);

	return CLZ(gbMask) - 1;
}

