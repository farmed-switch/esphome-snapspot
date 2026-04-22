

#ifndef _EndianPortable_h
#define _EndianPortable_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t Swap16NtoB(uint16_t inUInt16);
uint16_t Swap16BtoN(uint16_t inUInt16);

uint32_t Swap32NtoB(uint32_t inUInt32);
uint32_t Swap32BtoN(uint32_t inUInt32);

uint64_t Swap64BtoN(uint64_t inUInt64);
uint64_t Swap64NtoB(uint64_t inUInt64);

float SwapFloat32BtoN(float in);
float SwapFloat32NtoB(float in);

double SwapFloat64BtoN(double in);
double SwapFloat64NtoB(double in);

void Swap16(uint16_t * inUInt16);
void Swap24(uint8_t * inUInt24);
void Swap32(uint32_t * inUInt32);

#ifdef __cplusplus
}
#endif

#endif
