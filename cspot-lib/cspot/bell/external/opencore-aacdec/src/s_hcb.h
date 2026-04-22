

#ifndef S_HCB_H
#define S_HCB_H

#include    "pv_audio_type_defs.h"
#include    "s_huffman.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        Int     n;
        Int     dim;
        Int     mod;
        Int     off;
        Int     signed_cb;
    } Hcb;

#ifdef __cplusplus
}
#endif

#endif

