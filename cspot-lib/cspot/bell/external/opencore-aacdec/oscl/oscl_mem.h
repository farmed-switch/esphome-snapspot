

#ifndef OSCL_MEM_H
#define OSCL_MEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>
#define oscl_malloc malloc
#define oscl_free free
#define oscl_memset memset
#define oscl_memmove memmove
#define oscl_memcpy memcpy

#ifdef __cplusplus
}
#endif

#endif
