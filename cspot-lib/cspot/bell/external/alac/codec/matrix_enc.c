

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

void mix16( int16_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples, int32_t mixbits, int32_t mixres )
{
	int16_t	*	ip = in;
	int32_t			j;

	if ( mixres != 0 )
	{
		int32_t		mod = 1 << mixbits;
		int32_t		m2;

		m2 = mod - mixres;
		for ( j = 0; j < numSamples; j++ )
		{
			int32_t		l, r;

			l = (int32_t) ip[0];
			r = (int32_t) ip[1];
			ip += stride;
			u[j] = (mixres * l + m2 * r) >> mixbits;
			v[j] = l - r;
		}
	}
	else
	{

		for ( j = 0; j < numSamples; j++ )
		{
			u[j] = (int32_t) ip[0];
			v[j] = (int32_t) ip[1];
			ip += stride;
		}
	}
}

void mix20( uint8_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples, int32_t mixbits, int32_t mixres )
{
	int32_t		l, r;
	uint8_t *	ip = in;
	int32_t			j;

	if ( mixres != 0 )
	{

		int32_t		mod = 1 << mixbits;
		int32_t		m2 = mod - mixres;

		for ( j = 0; j < numSamples; j++ )
		{
			l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
			l = (l << 8) >> 12;
			ip += 3;

			r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
			r = (r << 8) >> 12;
			ip += (stride - 1) * 3;

			u[j] = (mixres * l + m2 * r) >> mixbits;
			v[j] = l - r;
		}
	}
	else
	{

		for ( j = 0; j < numSamples; j++ )
		{
			l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
			u[j] = (l << 8) >> 12;
			ip += 3;

			r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
			v[j] = (r << 8) >> 12;
			ip += (stride - 1) * 3;
		}
	}
}

void mix24( uint8_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples,
			int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted )
{
	int32_t		l, r;
	uint8_t *	ip = in;
	int32_t			shift = bytesShifted * 8;
	uint32_t	mask  = (1ul << shift) - 1;
	int32_t			j, k;

	if ( mixres != 0 )
	{

		int32_t		mod = 1 << mixbits;
		int32_t		m2 = mod - mixres;

		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				l = (l << 8) >> 8;
				ip += 3;

				r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				r = (r << 8) >> 8;
				ip += (stride - 1) * 3;

				shiftUV[k + 0] = (uint16_t)(l & mask);
				shiftUV[k + 1] = (uint16_t)(r & mask);

				l >>= shift;
				r >>= shift;

				u[j] = (mixres * l + m2 * r) >> mixbits;
				v[j] = l - r;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				l = (l << 8) >> 8;
				ip += 3;

				r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				r = (r << 8) >> 8;
				ip += (stride - 1) * 3;

				u[j] = (mixres * l + m2 * r) >> mixbits;
				v[j] = l - r;
			}
		}
	}
	else
	{

		if ( bytesShifted != 0 )
		{
			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				l = (l << 8) >> 8;
				ip += 3;

				r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				r = (r << 8) >> 8;
				ip += (stride - 1) * 3;

				shiftUV[k + 0] = (uint16_t)(l & mask);
				shiftUV[k + 1] = (uint16_t)(r & mask);

				l >>= shift;
				r >>= shift;

				u[j] = l;
				v[j] = r;
			}
		}
		else
		{
			for ( j = 0; j < numSamples; j++ )
			{
				l = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				u[j] = (l << 8) >> 8;
				ip += 3;

				r = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
				v[j] = (r << 8) >> 8;
				ip += (stride - 1) * 3;
			}
		}
	}
}

void mix32( int32_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples,
			int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted )
{
	int32_t	*	ip = in;
	int32_t			shift = bytesShifted * 8;
	uint32_t	mask  = (1ul << shift) - 1;
	int32_t		l, r;
	int32_t			j, k;

	if ( mixres != 0 )
	{
		int32_t		mod = 1 << mixbits;
		int32_t		m2;

		m2 = mod - mixres;
		for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
		{
			l = ip[0];
			r = ip[1];
			ip += stride;

			shiftUV[k + 0] = (uint16_t)(l & mask);
			shiftUV[k + 1] = (uint16_t)(r & mask);

			l >>= shift;
			r >>= shift;

			u[j] = (mixres * l + m2 * r) >> mixbits;
			v[j] = l - r;
		}
	}
	else
	{
		if ( bytesShifted == 0 )
		{

			for ( j = 0; j < numSamples; j++ )
			{
				u[j] = ip[0];
				v[j] = ip[1];
				ip += stride;
			}
		}
		else
		{

			for ( j = 0, k = 0; j < numSamples; j++, k += 2 )
			{
				l = ip[0];
				r = ip[1];
				ip += stride;

				shiftUV[k + 0] = (uint16_t)(l & mask);
				shiftUV[k + 1] = (uint16_t)(r & mask);

				l >>= shift;
				r >>= shift;

				u[j] = l;
				v[j] = r;
			}
		}
	}
}

void copy20ToPredictor( uint8_t * in, uint32_t stride, int32_t * out, int32_t numSamples )
{
	uint8_t *	ip = in;
	int32_t			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t			val;

		val = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
		out[j] = (val << 8) >> 12;
		ip += stride * 3;
	}
}

void copy24ToPredictor( uint8_t * in, uint32_t stride, int32_t * out, int32_t numSamples )
{
	uint8_t *	ip = in;
	int32_t			j;

	for ( j = 0; j < numSamples; j++ )
	{
		int32_t			val;

		val = (int32_t)( ((uint32_t)ip[HBYTE] << 16) | ((uint32_t)ip[MBYTE] << 8) | (uint32_t)ip[LBYTE] );
		out[j] = (val << 8) >> 8;
		ip += stride * 3;
	}
}
