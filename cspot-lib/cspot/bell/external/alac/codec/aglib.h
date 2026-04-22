

#ifndef AGLIB_H
#define AGLIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QBSHIFT 9
#define QB (1<<QBSHIFT)
#define PB0 40
#define MB0 10
#define KB0 14
#define MAX_RUN_DEFAULT 255

#define MMULSHIFT 2
#define MDENSHIFT (QBSHIFT - MMULSHIFT - 1)
#define MOFF ((1<<(MDENSHIFT-2)))

#define BITOFF 24

#define MAX_PREFIX_16			9
#define MAX_PREFIX_TOLONG_16	15
#define MAX_PREFIX_32			9

#define MAX_DATATYPE_BITS_16	16

typedef struct AGParamRec
{
    uint32_t mb, mb0, pb, kb, wb, qb;
    uint32_t fw, sw;

    uint32_t maxrun;

} AGParamRec, *AGParamRecPtr;

struct BitBuffer;

void	set_standard_ag_params(AGParamRecPtr params, uint32_t fullwidth, uint32_t sectorwidth);
void	set_ag_params(AGParamRecPtr params, uint32_t m, uint32_t p, uint32_t k, uint32_t f, uint32_t s, uint32_t maxrun);

int32_t		dyn_comp(AGParamRecPtr params, int32_t * pc, struct BitBuffer * bitstream, int32_t numSamples, int32_t bitSize, uint32_t * outNumBits);
int32_t		dyn_decomp(AGParamRecPtr params, struct BitBuffer * bitstream, int32_t * pc, int32_t numSamples, int32_t maxSize, uint32_t * outNumBits);

#ifdef __cplusplus
}
#endif

#endif
