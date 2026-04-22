

#ifndef SILK_TYPEDEF_H
#define SILK_TYPEDEF_H

#include "opus_types.h"
#include "opus_defines.h"

#ifndef FIXED_POINT
# include <float.h>
# define silk_float      float
# define silk_float_MAX  FLT_MAX
#endif

#define silk_int64_MAX   ((opus_int64)0x7FFFFFFFFFFFFFFFLL)
#define silk_int64_MIN   ((opus_int64)0x8000000000000000LL)
#define silk_int32_MAX   0x7FFFFFFF
#define silk_int32_MIN   ((opus_int32)0x80000000)
#define silk_int16_MAX   0x7FFF
#define silk_int16_MIN   ((opus_int16)0x8000)
#define silk_int8_MAX    0x7F
#define silk_int8_MIN    ((opus_int8)0x80)
#define silk_uint8_MAX   0xFF

#define silk_TRUE        1
#define silk_FALSE       0

#if (defined _WIN32 && !defined _WINCE && !defined(__GNUC__) && !defined(NO_ASSERTS))
# ifndef silk_assert
#  include <crtdbg.h>
#  define silk_assert(COND)   _ASSERTE(COND)
# endif
#else
# ifdef ENABLE_ASSERTIONS
#  include <stdio.h>
#  include <stdlib.h>
#define silk_fatal(str) _silk_fatal(str, __FILE__, __LINE__);
#ifdef __GNUC__
__attribute__((noreturn))
#endif
static OPUS_INLINE void _silk_fatal(const char *str, const char *file, int line)
{
   fprintf (stderr, "Fatal (internal) error in %s, line %d: %s\n", file, line, str);
#if defined(_MSC_VER)
   _set_abort_behavior( 0, _WRITE_ABORT_MSG);
#endif
   abort();
}
#  define silk_assert(COND) {if (!(COND)) {silk_fatal("assertion failed: " #COND);}}
# else
#  define silk_assert(COND)
# endif
#endif

#endif
