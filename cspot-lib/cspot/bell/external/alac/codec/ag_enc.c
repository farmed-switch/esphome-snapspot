

#include "aglib.h"
#include "ALACBitUtilities.h"
#include "EndianPortable.h"
#include "ALACAudioTypes.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if __GNUC__ && TARGET_OS_MAC
	#if __POWERPC__
		#include <ppc_intrinsics.h>
	#else
		#include <libkern/OSByteOrder.h>
	#endif
#endif

#define CODE_TO_LONG_MAXBITS	32
#define N_MAX_MEAN_CLAMP		0xffff
#define N_MEAN_CLAMP_VAL		0xffff
#define REPORT_VAL  40

#if __GNUC__
#define ALWAYS_INLINE		__attribute__((always_inline))
#else
#define ALWAYS_INLINE
#endif

static inline int32_t lead( int32_t m )
{
	long j;
	unsigned long c = (1ul << 31);

	for(j=0; j < 32; j++)
	{
		if((c & m) != 0)
			break;
		c >>= 1;
	}
	return (j);
}

#define arithmin(a, b) ((a) < (b) ? (a) : (b))

static inline int32_t ALWAYS_INLINE lg3a( int32_t x)
{
    int32_t result;

    x += 3;
    result = lead(x);

    return 31 - result;
}

static inline int32_t ALWAYS_INLINE abs_func( int32_t a )
{

	int32_t isneg  = a >> 31;
	int32_t xorval = a ^ isneg;
	int32_t result = xorval-isneg;

	return result;
}

static inline uint32_t ALWAYS_INLINE read32bit( uint8_t * buffer )
{

	uint32_t		value;

	value = ((uint32_t)buffer[0] << 24) | ((uint32_t)buffer[1] << 16) |
			 ((uint32_t)buffer[2] << 8) | (uint32_t)buffer[3];
	return value;
}

#if PRAGMA_MARK
#pragma mark -
#endif

static inline int32_t dyn_code(int32_t m, int32_t k, int32_t n, uint32_t *outNumBits)
{
	uint32_t 	div, mod, de;
	uint32_t	numBits;
	uint32_t	value;

	div = n/m;

	if(div >= MAX_PREFIX_16)
	{
		numBits = MAX_PREFIX_16 + MAX_DATATYPE_BITS_16;
		value = (((1<<MAX_PREFIX_16)-1)<<MAX_DATATYPE_BITS_16) + n;
	}
	else
	{
		mod = n%m;
		de = (mod == 0);
		numBits = div + k + 1 - de;
		value = (((1<<div)-1)<<(numBits-div)) + mod + 1 - de;

		if (numBits > MAX_PREFIX_16 + MAX_DATATYPE_BITS_16)
		{
		    numBits = MAX_PREFIX_16 + MAX_DATATYPE_BITS_16;
		    value = (((1<<MAX_PREFIX_16)-1)<<MAX_DATATYPE_BITS_16) + n;
		}
	}

	*outNumBits = numBits;

	return (int32_t) value;
}

static inline int32_t dyn_code_32bit(int32_t maxbits, uint32_t m, uint32_t k, uint32_t n, uint32_t *outNumBits, uint32_t *outValue, uint32_t *overflow, uint32_t *overflowbits)
{
	uint32_t 	div, mod, de;
	uint32_t	numBits;
	uint32_t	value;
	int32_t			didOverflow = 0;

	div = n/m;

	if (div < MAX_PREFIX_32)
	{
		mod = n - (m * div);

		de = (mod == 0);
		numBits = div + k + 1 - de;
		value = (((1<<div)-1)<<(numBits-div)) + mod + 1 - de;
		if (numBits > 25)
			goto codeasescape;
	}
	else
	{
codeasescape:
		numBits = MAX_PREFIX_32;
		value = (((1<<MAX_PREFIX_32)-1));
		*overflow = n;
		*overflowbits = maxbits;
		didOverflow = 1;
	}

	*outNumBits = numBits;
	*outValue = value;

	return didOverflow;
}

static inline void ALWAYS_INLINE dyn_jam_noDeref(unsigned char *out, uint32_t bitPos, uint32_t numBits, uint32_t value)
{
	uint32_t	*i = (uint32_t *)(out + (bitPos >> 3));
	uint32_t	mask;
	uint32_t	curr;
	uint32_t	shift;

	curr = *i;
	curr = Swap32NtoB( curr );

	shift = 32 - (bitPos & 7) - numBits;

	mask = ~0u >> (32 - numBits);
	mask <<= shift;

	value  = (value << shift) & mask;
	value |= curr & ~mask;

	*i = Swap32BtoN( value );
}

static inline void ALWAYS_INLINE dyn_jam_noDeref_large(unsigned char *out, uint32_t bitPos, uint32_t numBits, uint32_t value)
{
	uint32_t *	i = (uint32_t *)(out + (bitPos>>3));
	uint32_t	w;
	uint32_t	curr;
	uint32_t	mask;
	int32_t			shiftvalue = (32 - (bitPos&7) - numBits);

	curr = *i;
	curr = Swap32NtoB( curr );

	if (shiftvalue < 0)
	{
		uint8_t 	tailbyte;
		uint8_t 	*tailptr;

		w = value >> -shiftvalue;
		mask = ~0u >> -shiftvalue;
		w |= (curr & ~mask);

		tailptr = ((uint8_t *)i) + 4;
		tailbyte = (value << ((8+shiftvalue))) & 0xff;
		*tailptr = (uint8_t)tailbyte;
	}
	else
	{
		mask = ~0u >> (32 - numBits);
		mask <<= shiftvalue;

		w  = (value << shiftvalue) & mask;
		w |= curr & ~mask;
	}

	*i = Swap32BtoN( w );
}

int32_t dyn_comp( AGParamRecPtr params, int32_t * pc, BitBuffer * bitstream, int32_t numSamples, int32_t bitSize, uint32_t * outNumBits )
{
    unsigned char *		out;
    uint32_t		bitPos, startPos;
    uint32_t			m, k, n, c, mz, nz;
    uint32_t		numBits;
    uint32_t			value;
    int32_t				del, zmode;
	uint32_t		overflow, overflowbits;
    int32_t					status;

    uint32_t		mb, pb, kb, wb;
    int32_t					rowPos = 0;
    int32_t					rowSize = params->sw;
    int32_t					rowJump = (params->fw) - rowSize;
    int32_t *			inPtr = pc;

	*outNumBits = 0;
	RequireAction( (bitSize >= 1) && (bitSize <= 32), return kALAC_ParamError; );

	out = bitstream->cur;
	startPos = bitstream->bitIndex;
    bitPos = startPos;

    mb = params->mb = params->mb0;
    pb = params->pb;
    kb = params->kb;
    wb = params->wb;
    zmode = 0;

    c=0;
	status = ALAC_noErr;

    while (c < numSamples)
    {
        m  = mb >> QBSHIFT;
        k = lg3a(m);
        if ( k > kb)
        {
        	k = kb;
        }
        m = (1<<k)-1;

        del = *inPtr++;
        rowPos++;

        n = (abs_func(del) << 1) - ((del >> 31) & 1) - zmode;

		if ( dyn_code_32bit(bitSize, m, k, n, &numBits, &value, &overflow, &overflowbits) )
		{
			dyn_jam_noDeref(out, bitPos, numBits, value);
			bitPos += numBits;
			dyn_jam_noDeref_large(out, bitPos, overflowbits, overflow);
			bitPos += overflowbits;
		}
		else
		{
			dyn_jam_noDeref(out, bitPos, numBits, value);
			bitPos += numBits;
		}

        c++;
        if ( rowPos >= rowSize)
        {
        	rowPos = 0;
        	inPtr += rowJump;
        }

        mb = pb * (n + zmode) + mb - ((pb *mb)>>QBSHIFT);

		if (n > N_MAX_MEAN_CLAMP)
			mb = N_MEAN_CLAMP_VAL;

        zmode = 0;

        RequireAction(c <= numSamples, status = kALAC_ParamError; goto Exit; );

        if (((mb << MMULSHIFT) < QB) && (c < numSamples))
        {
            zmode = 1;
            nz = 0;

            while(c<numSamples && *inPtr == 0)
            {

                ++inPtr;
                ++nz;
                ++c;
                if ( ++rowPos >= rowSize)
                {
                	rowPos = 0;
                	inPtr += rowJump;
                }

                if(nz >= 65535)
                {
                	zmode = 0;
                	break;
                }
            }

            k = lead(mb) - BITOFF+((mb+MOFF)>>MDENSHIFT);
            mz = ((1<<k)-1) & wb;

            value = dyn_code(mz, k, nz, &numBits);
            dyn_jam_noDeref(out, bitPos, numBits, value);
            bitPos += numBits;

            mb = 0;
        }
    }

    *outNumBits = (bitPos - startPos);
	BitBufferAdvance( bitstream, *outNumBits );

Exit:
	return status;
}
