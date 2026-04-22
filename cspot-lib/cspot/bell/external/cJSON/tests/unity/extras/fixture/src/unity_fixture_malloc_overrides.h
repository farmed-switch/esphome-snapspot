

#ifndef UNITY_FIXTURE_MALLOC_OVERRIDES_H_
#define UNITY_FIXTURE_MALLOC_OVERRIDES_H_

#include <stddef.h>

#ifdef UNITY_EXCLUDE_STDLIB_MALLOC

    #ifndef UNITY_INTERNAL_HEAP_SIZE_BYTES
    #define UNITY_INTERNAL_HEAP_SIZE_BYTES 256
    #endif
#endif

#if !defined(UNITY_FIXTURE_MALLOC) || !defined(UNITY_FIXTURE_FREE)
    #include <stdlib.h>
    #define UNITY_FIXTURE_MALLOC(size) malloc(size)
    #define UNITY_FIXTURE_FREE(ptr)    free(ptr)
#else
    extern void* UNITY_FIXTURE_MALLOC(size_t size);
    extern void UNITY_FIXTURE_FREE(void* ptr);
#endif

#define malloc  unity_malloc
#define calloc  unity_calloc
#define realloc unity_realloc
#define free    unity_free

void* unity_malloc(size_t size);
void* unity_calloc(size_t num, size_t size);
void* unity_realloc(void * oldMem, size_t size);
void unity_free(void * mem);

#endif
