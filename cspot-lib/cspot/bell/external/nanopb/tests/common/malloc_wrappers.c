

#include "malloc_wrappers.h"
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define GUARD_SIZE (sizeof(size_t)*3)
#define PREFIX_SIZE (sizeof(size_t)*2)
#define CHECK1 ((size_t)0xDEADBEEF)
#define CHECK2 ((size_t)0x600DCAFE)

#ifndef MAX_ALLOC_BYTES
#define MAX_ALLOC_BYTES 16*1024*1024
#endif

#ifndef DEBUG_MALLOC
#define DEBUG_MALLOC 0
#endif

static size_t g_alloc_count = 0;
static size_t g_alloc_bytes = 0;
static size_t g_max_alloc_bytes = MAX_ALLOC_BYTES;

#ifdef LLVMFUZZER

static size_t round_blocksize(size_t size)
{
    if (size < 256)
    {
        return size;
    }
    else
    {
        return (size + 1023) / 1024 * 1024;
    }
}
#else
static size_t round_blocksize(size_t size)
{
    return size;
}
#endif

void* malloc_with_check(size_t size)
{
    char *buf = NULL;

    if (size <= g_max_alloc_bytes - g_alloc_bytes)
    {
        buf = malloc(round_blocksize(size + GUARD_SIZE));
    }

    if (buf)
    {
        ((size_t*)buf)[0] = size;
        ((size_t*)buf)[1] = CHECK1;
        ((size_t*)(buf + size))[2] = CHECK2;
        g_alloc_count++;
        g_alloc_bytes += size;
        if (DEBUG_MALLOC) fprintf(stderr, "Alloc 0x%04x/%u\n", (unsigned)(uintptr_t)(buf + PREFIX_SIZE), (unsigned)size);
        return buf + PREFIX_SIZE;
    }
    else
    {
        if (DEBUG_MALLOC) fprintf(stderr, "malloc(%u) failed\n", (unsigned)size);
        return NULL;
    }
}

void free_with_check(void *mem)
{
    if (mem)
    {
        char *buf = (char*)mem - PREFIX_SIZE;
        size_t size = ((size_t*)buf)[0];
        if (DEBUG_MALLOC) fprintf(stderr, "Release 0x%04x/%u\n", (unsigned)(uintptr_t)mem, (unsigned)size);
        assert(((size_t*)buf)[1] == CHECK1);
        assert(((size_t*)(buf + size))[2] == CHECK2);
        assert(g_alloc_count > 0);
        assert(g_alloc_bytes >= size);
        ((size_t*)buf)[1] = 0;
        ((size_t*)(buf + size))[2] = 0;
        g_alloc_count--;
        g_alloc_bytes -= size;
        free(buf);
    }
}

void* realloc_with_check(void *ptr, size_t size)
{
    if (!ptr && size)
    {

        return malloc_with_check(size);
    }
    else if (ptr && size)
    {

        char *buf = (char*)ptr - PREFIX_SIZE;
        size_t oldsize = ((size_t*)buf)[0];
        assert(((size_t*)buf)[1] == CHECK1);
        assert(((size_t*)(buf + oldsize))[2] == CHECK2);
        assert(g_alloc_count > 0);
        assert(g_alloc_bytes >= oldsize);

        if (size <= g_max_alloc_bytes - (g_alloc_bytes - oldsize))
        {
            size_t new_rounded = round_blocksize(size + GUARD_SIZE);
            size_t old_rounded = round_blocksize(oldsize + GUARD_SIZE);

            if (new_rounded != old_rounded)
            {
                buf = realloc(buf, new_rounded);
            }
        }
        else
        {
            buf = NULL;
        }

        if (!buf)
        {
            if (DEBUG_MALLOC) fprintf(stderr, "Realloc 0x%04x/%u to %u failed\n", (unsigned)(uintptr_t)ptr, (unsigned)oldsize, (unsigned)size);
            return NULL;
        }

        ((size_t*)buf)[0] = size;
        ((size_t*)buf)[1] = CHECK1;
        ((size_t*)(buf + size))[2] = CHECK2;
        g_alloc_bytes -= oldsize;
        g_alloc_bytes += size;

        if (DEBUG_MALLOC) fprintf(stderr, "Realloc 0x%04x/%u to 0x%04x/%u\n", (unsigned)(uintptr_t)ptr, (unsigned)oldsize, (unsigned)(uintptr_t)(buf + PREFIX_SIZE), (unsigned)size);
        return buf + PREFIX_SIZE;
    }
    else if (ptr && !size)
    {

        free_with_check(ptr);
        return NULL;
    }
    else
    {

        return NULL;
    }
}

size_t get_alloc_count()
{
    return g_alloc_count;
}

size_t get_allocation_size(const void *mem)
{
    char *buf = (char*)mem - PREFIX_SIZE;
    return ((size_t*)buf)[0];
}

size_t get_alloc_bytes()
{
    return g_alloc_bytes;
}

void set_max_alloc_bytes(size_t max_bytes)
{
    g_max_alloc_bytes = max_bytes;
}

size_t get_max_alloc_bytes()
{
    return g_max_alloc_bytes;
}
