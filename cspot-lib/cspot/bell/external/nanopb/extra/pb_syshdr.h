

#ifndef _PB_SYSHDR_H_
#define _PB_SYSHDR_H_

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef int8_t int_least8_t;
typedef uint8_t uint_least8_t;
typedef uint8_t uint_fast8_t;
typedef int16_t int_least16_t;
typedef uint16_t uint_least16_t;
#endif

#ifdef HAVE_STDDEF_H
#include <stddef.h>
#else

typedef uint32_t size_t;
#define offsetof(st, m) ((size_t)(&((st *)0)->m))

#ifndef NULL
#define NULL 0
#endif

#endif

#ifdef HAVE_STDBOOL_H
#include <stdbool.h>
#else

#ifndef __cplusplus
typedef int bool;
#define false 0
#define true 1
#endif

#endif

#ifdef PB_ENABLE_MALLOC
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
void *realloc(void *ptr, size_t size);
void free(void *ptr);
#endif
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else

static size_t strlen( const char * s )
{
    size_t rc = 0;
    while ( s[rc] )
    {
        ++rc;
    }
    return rc;
}

static void * memcpy( void *s1, const void *s2, size_t n )
{
    char * dest = (char *) s1;
    const char * src = (const char *) s2;
    while ( n-- )
    {
        *dest++ = *src++;
    }
    return s1;
}

static void * memset( void * s, int c, size_t n )
{
    unsigned char * p = (unsigned char *) s;
    while ( n-- )
    {
        *p++ = (unsigned char) c;
    }
    return s;
}
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#else
#define CHAR_BIT 8
#endif

#endif
