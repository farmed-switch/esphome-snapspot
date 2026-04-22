

#include <stdio.h>
#include "ALACBitUtilities.h"

void BitBufferInit( BitBuffer * bits, uint8_t * buffer, uint32_t byteSize )
{
	bits->cur		= buffer;
	bits->end		= bits->cur + byteSize;
	bits->bitIndex	= 0;
	bits->byteSize	= byteSize;
}

uint32_t BitBufferRead( BitBuffer * bits, uint8_t numBits )
{
	uint32_t		returnBits;

	returnBits = ((uint32_t)bits->cur[0] << 16) | ((uint32_t)bits->cur[1] << 8) | ((uint32_t)bits->cur[2]);
	returnBits = returnBits << bits->bitIndex;
	returnBits &= 0x00FFFFFF;

	bits->bitIndex += numBits;

	returnBits = returnBits >> (24 - numBits);

	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;

	return returnBits;
}

uint8_t BitBufferReadSmall( BitBuffer * bits, uint8_t numBits )
{
	uint16_t		returnBits;

	returnBits = (bits->cur[0] << 8) | bits->cur[1];
	returnBits = returnBits << bits->bitIndex;

	bits->bitIndex += numBits;

	returnBits = returnBits >> (16 - numBits);

	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;

	return (uint8_t)returnBits;
}

uint8_t BitBufferReadOne( BitBuffer * bits )
{
	uint8_t		returnBits;

	returnBits = (bits->cur[0] >> (7 - bits->bitIndex)) & 1;

	bits->bitIndex++;

	bits->cur		+= (bits->bitIndex >> 3);
	bits->bitIndex	&= 7;

	return returnBits;
}

uint32_t BitBufferPeek( BitBuffer * bits, uint8_t numBits )
{
	return ((((((uint32_t) bits->cur[0] << 16) | ((uint32_t) bits->cur[1] << 8) |
			((uint32_t) bits->cur[2])) << bits->bitIndex) & 0x00FFFFFF) >> (24 - numBits));
}

uint32_t BitBufferPeekOne( BitBuffer * bits )
{
	return ((bits->cur[0] >> (7 - bits->bitIndex)) & 1);
}

uint32_t BitBufferUnpackBERSize( BitBuffer * bits )
{
	uint32_t		size;
	uint8_t		tmp;

	for ( size = 0, tmp = 0x80u; tmp &= 0x80u; size = (size << 7u) | (tmp & 0x7fu) )
		tmp = (uint8_t) BitBufferReadSmall( bits, 8 );

	return size;
}

uint32_t BitBufferGetPosition( BitBuffer * bits )
{
	uint8_t *		begin;

	begin = bits->end - bits->byteSize;

	return ((uint32_t)(bits->cur - begin) * 8) + bits->bitIndex;
}

void BitBufferByteAlign( BitBuffer * bits, int32_t addZeros )
{

	if ( bits->bitIndex == 0 )
		return;

	if ( addZeros )
		BitBufferWrite( bits, 0, 8 - bits->bitIndex );
	else
		BitBufferAdvance( bits, 8 - bits->bitIndex );
}

void BitBufferAdvance( BitBuffer * bits, uint32_t numBits )
{
	if ( numBits )
	{
		bits->bitIndex += numBits;
		bits->cur += (bits->bitIndex >> 3);
		bits->bitIndex &= 7;
	}
}

void BitBufferRewind( BitBuffer * bits, uint32_t numBits )
{
	uint32_t	numBytes;

	if ( numBits == 0 )
		return;

	if ( bits->bitIndex >= numBits )
	{
		bits->bitIndex -= numBits;
		return;
	}

	numBits -= bits->bitIndex;
	bits->bitIndex = 0;

	numBytes	= numBits / 8;
	numBits		= numBits % 8;

	bits->cur -= numBytes;

	if ( numBits > 0 )
	{
		bits->bitIndex = 8 - numBits;
		bits->cur--;
	}

	if ( bits->cur < (bits->end - bits->byteSize) )
	{

		bits->cur		= (bits->end - bits->byteSize);
		bits->bitIndex	= 0;
	}
}

void BitBufferWrite( BitBuffer * bits, uint32_t bitValues, uint32_t numBits )
{
	uint32_t				invBitIndex;

	RequireAction( bits != nil, return; );
	RequireActionSilent( numBits > 0, return; );

	invBitIndex = 8 - bits->bitIndex;

	while ( numBits > 0 )
	{
		uint32_t		tmp;
		uint8_t		shift;
		uint8_t		mask;
		uint32_t		curNum;

		curNum = MIN( invBitIndex, numBits );

		tmp = bitValues >> (numBits - curNum);

		shift  = (uint8_t)(invBitIndex - curNum);
		mask   = 0xffu >> (8 - curNum);
		mask <<= shift;

		bits->cur[0] = (bits->cur[0] & ~mask) | (((uint8_t) tmp << shift)  & mask);
		numBits -= curNum;

		invBitIndex -= curNum;
		if ( invBitIndex == 0 )
		{
			invBitIndex = 8;
			bits->cur++;
		}
	}

	bits->bitIndex = 8 - invBitIndex;
}

void	BitBufferReset( BitBuffer * bits )

{
	bits->cur		= bits->end - bits->byteSize;
    bits->bitIndex	= 0;
}

#if PRAGMA_MARK
#pragma mark -
#endif
