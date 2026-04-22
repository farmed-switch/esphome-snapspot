

#include "matrixlib.h"
#include "ALACAudioTypes.h"

#if TARGET_RT_BIG_ENDIAN
	#define LBYTE	2
	#define MBYTE	1
	#define HBYTE	0
#else
	#define LBYTE	0
	#define MBYTE	1
	#define HBYTE	2
#endif

void unmix16( int32_t * u, int32_t * v, int16_t * out, uint32_t stride, int32_t numSamples, int32_t mixbits, int32_t mixres )
{
	int16_t *	op = out;
	int32_t 		j;

	if ( mixres != 0 )
	{

		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		l, r;

			l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
			r = l - v[j];

			op[0] = (int16_t) l;
			op[1] = (int16_t) r;
			op += stride;
		}
	}
	else
	{

		for ( j = 0; j < numSamples; j++ )
		{
			op[0] = (int16_t) u[j];
			op[1] = (int16_t) v[j];
			op += stride;
		}
	}
}

void unmix20( int32_t * u, int32_t * v, uint8_t * out, uint32_t stride, int32_t numSamples, int32_t mixbits, int32_t mixres )
{
	uint8_t *	op = out;
	int32_t 		j;

	if ( mixres != 0 )
	{

		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		l, r;

			l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
			r = l - v[j];

			l <<= 4;
			r <<= 4;

			op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
			op += 3;

			op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

			op += (stride - 1) * 3;
		}
	}
	else
	{

		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		val;

			val = u[j] << 4;
			op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
			op += 3;

			val = v[j] << 4;
			op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
			op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
			op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);

			op += (stride - 1) * 3;
		}
	}
}

void unmix24( int32_t * u, int32_t * v, uint8_t * out, uint32_t stride, int32_t numSamples,
				int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted )
{
	uint8_t *	op = out;
	int32_t			shift = bytesShifted * 8;
	int32_t		l, r;
	int32_t 		j, k;

	if ( mixres != 0 )
	{

		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
				r = l - v[j];

				l = (l << shift) | (uint32_t) shiftUV[k + 0];
				r = (r << shift) | (uint32_t) shiftUV[k + 1];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				l = u[j] + v[j] - ((mixres * v[j]) >> mixbits);
				r = l - v[j];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
	}
	else
	{

		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = u[j];
				r = v[j];

				l = (l << shift) | (uint32_t) shiftUV[k + 0];
				r = (r << shift) | (uint32_t) shiftUV[k + 1];

				op[HBYTE] = (uint8_t)((l >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((l >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((l >>  0) & 0xffu);
				op += 3;

				op[HBYTE] = (uint8_t)((r >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((r >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((r >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				int32_t		val;

				val = u[j];
				op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
				op += 3;

				val = v[j];
				op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
				op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
				op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);

				op += (stride - 1) * 3;
			}
		}
	}
}

void unmix32( int32_t * u, int32_t * v, int32_t * out, uint32_t stride, int32_t numSamples,
				int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted )
{
	int32_t *	op = out;
	int32_t			shift = bytesShifted * 8;
	int32_t		l, r;
	int32_t 		j, k;

	if ( mixres != 0 )
	{

		for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
		{
			int32_t		lt, rt;

			lt = u[j];
			rt = v[j];

			l = lt + rt - ((mixres * rt) >> mixbits);
			r = l - rt;

			op[0] = (l << shift) | (uint32_t) shiftUV[k + 0];
			op[1] = (r << shift) | (uint32_t) shiftUV[k + 1];
			op += stride;
		}
	}
	else
	{
		if ( bytesShifted == 0 )
		{

			for ( j = 0; j < numSamples; j++ )
			{
				op[0] = u[j];
				op[1] = v[j];
				op += stride;
			}
		}
		else
		{

			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				op[0] = (u[j] << shift) | (uint32_t) shiftUV[k + 0];
				op[1] = (v[j] << shift) | (uint32_t) shiftUV[k + 1];
				op += stride;
			}
		}
	}
}

void copyPredictorTo24( int32_t * in, uint8_t * out, uint32_t stride, int32_t numSamples )
{
	uint8_t *	op = out;
	int32_t			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
		op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo24Shift( int32_t * in, uint16_t * shift, uint8_t * out, uint32_t stride, int32_t numSamples, int32_t bytesShifted )
{
	uint8_t *	op = out;
	int32_t			shiftVal = bytesShifted * 8;
	int32_t			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		val = (val << shiftVal) | (uint32_t) shift[j];

		op[HBYTE] = (uint8_t)((val >> 16) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  8) & 0xffu);
		op[LBYTE] = (uint8_t)((val >>  0) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo20( int32_t * in, uint8_t * out, uint32_t stride, int32_t numSamples )
{
	uint8_t *	op = out;
	int32_t			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t		val = in[j];

		op[HBYTE] = (uint8_t)((val >> 12) & 0xffu);
		op[MBYTE] = (uint8_t)((val >>  4) & 0xffu);
		op[LBYTE] = (uint8_t)((val <<  4) & 0xffu);
		op += (stride * 3);
	}
}

void copyPredictorTo32( int32_t * in, int32_t * out, uint32_t stride, int32_t numSamples )
{
	int32_t			i, j;

	for ( i = 0, j = 0; i < numSamples; i++, j += stride )
		out[j] = in[i];
}

void copyPredictorTo32Shift( int32_t * in, uint16_t * shift, int32_t * out, uint32_t stride, int32_t numSamples, int32_t bytesShifted )
{
	int32_t *		op = out;
	uint32_t		shiftVal = bytesShifted * 8;
	int32_t				j;

	for ( j = 0; j < numSamples; j++ )
	{
		op[0] = (in[j] << shiftVal) | (uint32_t) shift[j];
		op += stride;
	}
}
