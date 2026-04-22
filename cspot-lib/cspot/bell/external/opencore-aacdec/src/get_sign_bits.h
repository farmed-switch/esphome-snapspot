

#ifndef GET_SIGN_BITS_H
#define GET_SIGN_BITS_H

#include "pv_audio_type_defs.h"
#include "ibstream.h"

void get_sign_bits(
    Int          q[],
    BITS        *pInputStream,
    const Int    q_len
);

#endif

