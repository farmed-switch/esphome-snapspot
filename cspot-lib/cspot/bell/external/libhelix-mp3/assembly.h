

#ifndef _ASSEMBLY_H
#define _ASSEMBLY_H

#if (defined _WIN32 && !defined _WIN32_WCE) || (defined __WINS__ && defined _SYMBIAN) || defined(_OPENWAVE_SIMULATOR) || defined(WINCE_EMULATOR)

#pragma warning( disable : 4035 )

static __inline int MULSHIFT32(int x, int y)
{
    __asm {
		mov		eax, x
	    imul	y
	    mov		eax, edx
	}
}

static __inline int FASTABS(int x)
{
	int sign;

	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;

	return x;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	}

	return numZeros;
}

#ifdef __CW32__
typedef long long Word64;
#else
typedef __int64 Word64;
#endif

static __inline Word64 MADD64(Word64 sum, int x, int y)
{
	unsigned int sumLo = ((unsigned int *)&sum)[0];
	int sumHi = ((int *)&sum)[1];

	__asm {
		mov		eax, x
		imul	y
		add		eax, sumLo
		adc		edx, sumHi
	}

}

static __inline Word64 SHL64(Word64 x, int n)
{
	unsigned int xLo = ((unsigned int *)&x)[0];
	int xHi = ((int *)&x)[1];
	unsigned char nb = (unsigned char)n;

	if (n < 32) {
		__asm {
			mov		edx, xHi
			mov		eax, xLo
			mov		cl, nb
			shld    edx, eax, cl
			shl     eax, cl
		}
	} else if (n < 64) {

		__asm {
			mov		edx, xLo
			mov		cl, nb
			xor     eax, eax
			shl     edx, cl
		}
	} else {
		__asm {
			xor		edx, edx
			xor		eax, eax
		}
	}
}

static __inline Word64 SAR64(Word64 x, int n)
{
	unsigned int xLo = ((unsigned int *)&x)[0];
	int xHi = ((int *)&x)[1];
	unsigned char nb = (unsigned char)n;

	if (n < 32) {
		__asm {
			mov		edx, xHi
			mov		eax, xLo
			mov		cl, nb
			shrd	eax, edx, cl
			sar		edx, cl
		}
	} else if (n < 64) {

		__asm {
			mov		edx, xHi
			mov		eax, xHi
			mov		cl, nb
			sar		edx, 31
			sar		eax, cl
		}
	} else {
		__asm {
			sar		xHi, 31
			mov		eax, xHi
			mov		edx, xHi
		}
	}
}

#elif (defined _WIN32) && (defined _WIN32_WCE)

#define MULSHIFT32	xmp3_MULSHIFT32
int MULSHIFT32(int x, int y);

static __inline int FASTABS(int x)
{
	int sign;

	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;

	return x;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	}

	return numZeros;
}

#elif defined XXXARM_ADS

static __inline int MULSHIFT32(int x, int y)
{

    int zlow;
    __asm {
    	smull zlow,y,x,y
   	}

    return y;
}

static __inline int FASTABS(int x)
{
	int t=0;

	__asm {
		eor	t, x, x, asr #31
		sub	t, t, x, asr #31
	}

	return t;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	}

	return numZeros;
}

#elif defined(__GNUC__) && defined(XXXX__thumb__)

static __inline int MULSHIFT32(int x, int y)
{

    int zlow;
    __asm__ volatile ("smull %0,%1,%2,%3" : "=&r" (zlow), "=r" (y) : "r" (x), "1" (y)) ;
    return y;
}

#include <stdlib.h>
static __inline int FASTABS(int x)
{
  return abs(x);
}

static __inline int CLZ(int x)
{
  return __builtin_clz(x);
}

static __inline Word64 xMADD64(Word64 sum, int x, int y)
{
   return (sum + ((int64_t)x * y));
}
static __inline Word64 xHL64(Word64 x, int n)
{
  return x << n;
}

static __inline Word64 xSAR64(Word64 x, int n)
{
  return x >> n;
}

#elif defined(__arm__)

#if defined(ARM7DI)

typedef long long Word64;

static __inline int MULSHIFT32(int x, int y) {
	return x * y;
}

#else

static __inline Word64 SAR64(Word64 x, int n) {
	return x >>= n;
}

typedef union _U64 {
	Word64 w64;
	struct {

		unsigned int lo32;
		signed int   hi32;
	} r;
} U64;

static __inline Word64 MADD64(Word64 sum64, int x, int y)
{
	sum64 += (Word64)x * (Word64)y;

	return sum64;
}

static __inline int MULSHIFT32(int x, int y)
{

	int zlow;
	__asm__ volatile ("smull %0,%1,%2,%3" : "=&r" (zlow), "=r" (y) : "r" (x), "1" (y)) ;

	return y;
}

#endif

static __inline int FASTABS(int x)
{
	int t=0;

	__asm__ volatile (
		"eor %0,%2,%2, asr #31;"
		"sub %0,%1,%2, asr #31;"
		: "=&r" (t)
		: "0" (t), "r" (x)
	 );

	return t;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return (sizeof(int) * 8);

	numZeros = 0;
	while (!(x & 0x80000000)) {
		numZeros++;
		x <<= 1;
	}

	return numZeros;
}

#elif defined(__APPLE__) || defined(ESP_PLATFORM) || defined(__amd64__)

static __inline int FASTABS(int x)
{
  int sign;

  sign = x >> (sizeof(int) * 8 - 1);
  x ^= sign;
  x -= sign;

  return x;
}

static __inline int CLZ(int x)
{
  int numZeros;

  if (!x)
    return (sizeof(int) * 8);

  numZeros = 0;
  while (!(x & 0x80000000)) {
    numZeros++;
    x <<= 1;
  }

  return numZeros;
}

static __inline Word64 MADD64(Word64 sum64, int x, int y)
{
    sum64 += (Word64)x * (Word64)y;
    return sum64;
}

static __inline__ int MULSHIFT32(int x, int y)
{
    int z;

    z = (Word64)x * (Word64)y >> 32;

        return z;
}

static __inline Word64 SAR64(Word64 x, int n)
{
  return x >> n;
}

#else

#error Unsupported platform in assembly.h

#endif

#endif
