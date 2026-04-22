

#ifndef __ALACBITUTILITIES_H
#define __ALACBITUTILITIES_H

#include <stdint.h>

#ifndef MIN
#define MIN(x, y) 			( (x)<(y) ?(x) :(y) )
#endif
#ifndef MAX
#define MAX(x, y) 			( (x)>(y) ?(x): (y) )
#endif

#ifndef nil
#define nil NULL
#endif

#define RequireAction(condition, action)			if (!(condition)) { action }
#define RequireActionSilent(condition, action)			if (!(condition)) { action }
#define RequireNoErr(condition, action)			if ((condition)) { action }

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    ALAC_noErr = 0
};

typedef enum
{

    ID_SCE = 0,
    ID_CPE = 1,
    ID_CCE = 2,
    ID_LFE = 3,
    ID_DSE = 4,
    ID_PCE = 5,
    ID_FIL = 6,
    ID_END = 7
} ELEMENT_TYPE;

typedef struct BitBuffer
{
	uint8_t *		cur;
	uint8_t *		end;
	uint32_t		bitIndex;
	uint32_t		byteSize;

} BitBuffer;

void	BitBufferInit( BitBuffer * bits, uint8_t * buffer, uint32_t byteSize );
uint32_t	BitBufferRead( BitBuffer * bits, uint8_t numBits );
uint8_t	BitBufferReadSmall( BitBuffer * bits, uint8_t numBits );
uint8_t	BitBufferReadOne( BitBuffer * bits );
uint32_t	BitBufferPeek( BitBuffer * bits, uint8_t numBits );
uint32_t	BitBufferPeekOne( BitBuffer * bits );
uint32_t	BitBufferUnpackBERSize( BitBuffer * bits );
uint32_t	BitBufferGetPosition( BitBuffer * bits );
void	BitBufferByteAlign( BitBuffer * bits, int32_t addZeros );
void	BitBufferAdvance( BitBuffer * bits, uint32_t numBits );
void	BitBufferRewind( BitBuffer * bits, uint32_t numBits );
void	BitBufferWrite( BitBuffer * bits, uint32_t value, uint32_t numBits );
void	BitBufferReset( BitBuffer * bits);

#ifdef __cplusplus
}
#endif

#endif
