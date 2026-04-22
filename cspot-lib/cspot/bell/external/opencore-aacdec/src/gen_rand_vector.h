

#ifndef gen_rand_vector_H
#define gen_rand_vector_H

#include "pv_audio_type_defs.h"

Int gen_rand_vector(
    Int32  random_array[],
    const Int    band_length,
    Int32 *pSeed,
    const Int    power_scale);

#endif
