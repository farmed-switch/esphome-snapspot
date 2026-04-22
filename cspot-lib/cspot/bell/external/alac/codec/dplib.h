

#ifndef __DPLIB_H__
#define __DPLIB_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DENSHIFT_MAX  15
#define DENSHIFT_DEFAULT 9
#define AINIT 38
#define BINIT (-29)
#define CINIT (-2)
#define NUMCOEPAIRS 16

void init_coefs( int16_t * coefs, uint32_t denshift, int32_t numPairs );
void copy_coefs( int16_t * srcCoefs, int16_t * dstCoefs, int32_t numPairs );

void pc_block( int32_t * in, int32_t * pc, int32_t num, int16_t * coefs, int32_t numactive, uint32_t chanbits, uint32_t denshift );
void unpc_block( int32_t * pc, int32_t * out, int32_t num, int16_t * coefs, int32_t numactive, uint32_t chanbits, uint32_t denshift );

#ifdef __cplusplus
}
#endif

#endif
