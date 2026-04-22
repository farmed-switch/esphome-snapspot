

#ifndef FWD_SHORT_COMPLEX_ROT_H
#define FWD_SHORT_COMPLEX_ROT_H

#include "pv_audio_type_defs.h"

#define FWD_SHORT_CX_ROT_LENGTH             64
#define TWICE_FWD_SHORT_CX_ROT_LENGTH       (FWD_SHORT_CX_ROT_LENGTH<<1)
#define TWICE_FWD_SHORT_CX_ROT_LENGTH_m_1   ((FWD_SHORT_CX_ROT_LENGTH<<1) - 1)
#define FOUR_FWD_SHORT_CX_ROT_LENGTH_m_1    ((FWD_SHORT_CX_ROT_LENGTH<<2) - 1)

Int fwd_short_complex_rot(
    Int32 *Data_in,
    Int32 *Data_out,
    Int32  max);

#endif
