

#ifndef AAC_MEM_FUNCS_H
#define AAC_MEM_FUNCS_H

#include "pv_audio_type_defs.h"
#include "oscl_mem.h"

#define pv_memset(to, c, n)         oscl_memset(to, c, n)

#define pv_memcpy(to, from, n)      oscl_memcpy(to, from, n)
#define pv_memmove(to, from, n)     oscl_memmove(to, from, n)
#define pv_memcmp(p, q, n)          oscl_memcmp(p, q, n)

#endif
