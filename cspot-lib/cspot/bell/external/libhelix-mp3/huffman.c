

#include "coder.h"
#define PGM_READ_UNALIGNED 0

#define GetMaxbits(x)   ((int)( (((unsigned short)(x)) >>  0) & 0x000f))
#define GetHLen(x)      ((int)( (((unsigned short)(x)) >> 12) & 0x000f))
#define GetCWY(x)       ((int)( (((unsigned short)(x)) >>  8) & 0x000f))
#define GetCWX(x)       ((int)( (((unsigned short)(x)) >>  4) & 0x000f))
#define GetSignBits(x)  ((int)( (((unsigned short)(x)) >>  0) & 0x000f))

#define GetHLenQ(x)     ((int)( (((unsigned char)(x)) >> 4) & 0x0f))
#define GetCWVQ(x)      ((int)( (((unsigned char)(x)) >> 3) & 0x01))
#define GetCWWQ(x)      ((int)( (((unsigned char)(x)) >> 2) & 0x01))
#define GetCWXQ(x)      ((int)( (((unsigned char)(x)) >> 1) & 0x01))
#define GetCWYQ(x)      ((int)( (((unsigned char)(x)) >> 0) & 0x01))

#define ApplySign(x, s)	{ (x) |= ((s) & 0x80000000); }

static int DecodeHuffmanPairs(int *xy, int nVals, int tabIdx, int bitsLeft, unsigned char *buf, int bitOffset)
{
	int i, x, y;
	int cachedBits, padBits, len, startBits, linBits, maxBits, minBits;
	HuffTabType tabType;
	unsigned short cw, *tBase, *tCurr;
	unsigned int cache;

	if(nVals <= 0)
		return 0;

	if (bitsLeft < 0)
		return -1;
	startBits = bitsLeft;

	tBase = (unsigned short *)(huffTable + huffTabOffset[tabIdx]);
	linBits = huffTabLookup[tabIdx].linBits;
	tabType = huffTabLookup[tabIdx].tabType;

	ASSERT(!(nVals & 0x01));
	ASSERT(tabIdx < HUFF_PAIRTABS);
	ASSERT(tabIdx >= 0);
	ASSERT(tabType != invalidTab);

	cache = 0;
	cachedBits = (8 - bitOffset) & 0x07;
	if (cachedBits)
		cache = (unsigned int)(*buf++) << (32 - cachedBits);
	bitsLeft -= cachedBits;

	if (tabType == noBits) {

		for (i = 0; i < nVals; i+=2) {
			xy[i+0] = 0;
			xy[i+1] = 0;
		}
		return 0;
	} else if (tabType == oneShot) {

		maxBits = GetMaxbits(pgm_read_word(&tBase[0]));
		tBase++;
		padBits = 0;
		while (nVals > 0) {

			if (bitsLeft >= 16) {

				cache |= (unsigned int)(*buf++) << (24 - cachedBits);
				cache |= (unsigned int)(*buf++) << (16 - cachedBits);
				cachedBits += 16;
				bitsLeft -= 16;
			} else {

				if (cachedBits + bitsLeft <= 0)	return -1;
				if (bitsLeft > 0)	cache |= (unsigned int)(*buf++) << (24 - cachedBits);
				if (bitsLeft > 8)	cache |= (unsigned int)(*buf++) << (16 - cachedBits);
				cachedBits += bitsLeft;
				bitsLeft = 0;

				cache &= (signed int)0x80000000 >> (cachedBits - 1);
				padBits = 11;
				cachedBits += padBits;
			}

			while (nVals > 0 && cachedBits >= 11 ) {
				cw = pgm_read_word(&tBase[cache >> (32 - maxBits)]);
				len = GetHLen(cw);
				cachedBits -= len;
				cache <<= len;

				x = GetCWX(cw);		if (x)	{ApplySign(x, cache); cache <<= 1; cachedBits--;}
				y = GetCWY(cw);		if (y)	{ApplySign(y, cache); cache <<= 1; cachedBits--;}

				if (cachedBits < padBits)
					return -1;

				*xy++ = x;
				*xy++ = y;
				nVals -= 2;
			}
		}
		bitsLeft += (cachedBits - padBits);
		return (startBits - bitsLeft);
	} else if (tabType == loopLinbits || tabType == loopNoLinbits) {
		tCurr = tBase;
		padBits = 0;
		while (nVals > 0) {

			if (bitsLeft >= 16) {

				cache |= (unsigned int)(*buf++) << (24 - cachedBits);
				cache |= (unsigned int)(*buf++) << (16 - cachedBits);
				cachedBits += 16;
				bitsLeft -= 16;
			} else {

				if (cachedBits + bitsLeft <= 0)	return -1;
				if (bitsLeft > 0)	cache |= (unsigned int)(*buf++) << (24 - cachedBits);
				if (bitsLeft > 8)	cache |= (unsigned int)(*buf++) << (16 - cachedBits);
				cachedBits += bitsLeft;
				bitsLeft = 0;

				cache &= (signed int)0x80000000 >> (cachedBits - 1);
				padBits = 11;
				cachedBits += padBits;
			}

			while (nVals > 0 && cachedBits >= 11 ) {
				maxBits = GetMaxbits(pgm_read_word(&tCurr[0]));
				cw = pgm_read_word(&tCurr[(cache >> (32 - maxBits)) + 1]);
				len = GetHLen(cw);
				if (!len) {
					cachedBits -= maxBits;
					cache <<= maxBits;
					tCurr += cw;
					continue;
				}
				cachedBits -= len;
				cache <<= len;

				x = GetCWX(cw);
				y = GetCWY(cw);

				if (x == 15 && tabType == loopLinbits) {
					minBits = linBits + 1 + (y ? 1 : 0);
					if (cachedBits + bitsLeft < minBits)
						return -1;
					while (cachedBits < minBits) {
						cache |= (unsigned int)(*buf++) << (24 - cachedBits);
						cachedBits += 8;
						bitsLeft -= 8;
					}
					if (bitsLeft < 0) {
						cachedBits += bitsLeft;
						bitsLeft = 0;
						cache &= (signed int)0x80000000 >> (cachedBits - 1);
					}
					x += (int)(cache >> (32 - linBits));
					cachedBits -= linBits;
					cache <<= linBits;
				}
				if (x)	{ApplySign(x, cache); cache <<= 1; cachedBits--;}

				if (y == 15 && tabType == loopLinbits) {
					minBits = linBits + 1;
					if (cachedBits + bitsLeft < minBits)
						return -1;
					while (cachedBits < minBits) {
						cache |= (unsigned int)(*buf++) << (24 - cachedBits);
						cachedBits += 8;
						bitsLeft -= 8;
					}
					if (bitsLeft < 0) {
						cachedBits += bitsLeft;
						bitsLeft = 0;
						cache &= (signed int)0x80000000 >> (cachedBits - 1);
					}
					y += (int)(cache >> (32 - linBits));
					cachedBits -= linBits;
					cache <<= linBits;
				}
				if (y)	{ApplySign(y, cache); cache <<= 1; cachedBits--;}

				if (cachedBits < padBits)
					return -1;

				*xy++ = x;
				*xy++ = y;
				nVals -= 2;
				tCurr = tBase;
			}
		}
		bitsLeft += (cachedBits - padBits);
		return (startBits - bitsLeft);
	}

	return -1;
}

static int DecodeHuffmanQuads(int *vwxy, int nVals, int tabIdx, int bitsLeft, unsigned char *buf, int bitOffset)
{
	int i, v, w, x, y;
	int len, maxBits, cachedBits, padBits;
	unsigned int cache;
	unsigned char cw, *tBase;

	if (bitsLeft <= 0)
		return 0;

	tBase = (unsigned char *)quadTable + quadTabOffset[tabIdx];
	maxBits = quadTabMaxBits[tabIdx];

	cache = 0;
	cachedBits = (8 - bitOffset) & 0x07;
	if (cachedBits)
		cache = (unsigned int)(*buf++) << (32 - cachedBits);
	bitsLeft -= cachedBits;

	i = padBits = 0;
	while (i < (nVals - 3)) {

		if (bitsLeft >= 16) {

			cache |= (unsigned int)(*buf++) << (24 - cachedBits);
			cache |= (unsigned int)(*buf++) << (16 - cachedBits);
			cachedBits += 16;
			bitsLeft -= 16;
		} else {

			if (cachedBits + bitsLeft <= 0) return i;
			if (bitsLeft > 0)	cache |= (unsigned int)(*buf++) << (24 - cachedBits);
			if (bitsLeft > 8)	cache |= (unsigned int)(*buf++) << (16 - cachedBits);
			cachedBits += bitsLeft;
			bitsLeft = 0;

			cache &= (signed int)0x80000000 >> (cachedBits - 1);
			padBits = 10;
			cachedBits += padBits;
		}

		while (i < (nVals - 3) && cachedBits >= 10 ) {
			cw = pgm_read_byte(&tBase[cache >> (32 - maxBits)]);
			len = GetHLenQ(cw);
			cachedBits -= len;
			cache <<= len;

			v = GetCWVQ(cw);	if(v) {ApplySign(v, cache); cache <<= 1; cachedBits--;}
			w = GetCWWQ(cw);	if(w) {ApplySign(w, cache); cache <<= 1; cachedBits--;}
			x = GetCWXQ(cw);	if(x) {ApplySign(x, cache); cache <<= 1; cachedBits--;}
			y = GetCWYQ(cw);	if(y) {ApplySign(y, cache); cache <<= 1; cachedBits--;}

			if (cachedBits < padBits)
				return i;

			*vwxy++ = v;
			*vwxy++ = w;
			*vwxy++ = x;
			*vwxy++ = y;
			i += 4;
		}
	}

	return i;
}

  int DecodeHuffman(MP3DecInfo *mp3DecInfo, unsigned char *buf, int *bitOffset, int huffBlockBits, int gr, int ch)
{
	int r1Start, r2Start, rEnd[4];
	int i, w, bitsUsed, bitsLeft;
	unsigned char *startBuf = buf;

	FrameHeader *fh;
	SideInfo *si;
	SideInfoSub *sis;

	HuffmanInfo *hi;

	if (!mp3DecInfo || !mp3DecInfo->FrameHeaderPS || !mp3DecInfo->SideInfoPS || !mp3DecInfo->ScaleFactorInfoPS || !mp3DecInfo->HuffmanInfoPS)
		return -1;

	fh = ((FrameHeader *)(mp3DecInfo->FrameHeaderPS));
	si = ((SideInfo *)(mp3DecInfo->SideInfoPS));
	sis = &si->sis[gr][ch];

	hi = (HuffmanInfo*)(mp3DecInfo->HuffmanInfoPS);

	if (huffBlockBits < 0)
		return -1;

	if (sis->winSwitchFlag && sis->blockType == 2) {
		if (sis->mixedBlock == 0) {
			r1Start = fh->sfBand->s[(sis->region0Count + 1)/3] * 3;
		} else {
			if (fh->ver == MPEG1) {
				r1Start = fh->sfBand->l[sis->region0Count + 1];
			} else {

				w = fh->sfBand->s[4] - fh->sfBand->s[3];
				r1Start = fh->sfBand->l[6] + 2*w;
			}
		}
		r2Start = MAX_NSAMP;
	} else {
		r1Start = fh->sfBand->l[sis->region0Count + 1];
		r2Start = fh->sfBand->l[sis->region0Count + 1 + sis->region1Count + 1];
	}

	rEnd[3] = MIN(MAX_NSAMP, 2 * sis->nBigvals);
	rEnd[2] = MIN(r2Start, rEnd[3]);
	rEnd[1] = MIN(r1Start, rEnd[3]);
	rEnd[0] = 0;

	hi->nonZeroBound[ch] = rEnd[3];

	bitsLeft = huffBlockBits;
	for (i = 0; i < 3; i++) {
		bitsUsed = DecodeHuffmanPairs(hi->huffDecBuf[ch] + rEnd[i], rEnd[i+1] - rEnd[i], sis->tableSelect[i], bitsLeft, buf, *bitOffset);
		if (bitsUsed < 0 || bitsUsed > bitsLeft)
			return -1;

		buf += (bitsUsed + *bitOffset) >> 3;
		*bitOffset = (bitsUsed + *bitOffset) & 0x07;
		bitsLeft -= bitsUsed;
	}

	hi->nonZeroBound[ch] += DecodeHuffmanQuads(hi->huffDecBuf[ch] + rEnd[3], MAX_NSAMP - rEnd[3], sis->count1TableSelect, bitsLeft, buf, *bitOffset);

	ASSERT(hi->nonZeroBound[ch] <= MAX_NSAMP);
	for (i = hi->nonZeroBound[ch]; i < MAX_NSAMP; i++)
		hi->huffDecBuf[ch][i] = 0;

	buf += (bitsLeft + *bitOffset) >> 3;
	*bitOffset = (bitsLeft + *bitOffset) & 0x07;

	return (buf - startBuf);
}

