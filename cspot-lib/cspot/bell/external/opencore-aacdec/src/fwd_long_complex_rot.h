

#ifndef FWD_LONG_COMPLEX_ROT_H
#define FWD_LONG_COMPLEX_ROT_H

#include "pv_audio_type_defs.h"

#define FWD_LONG_CX_ROT_LENGTH              256
#define TWICE_FWD_LONG_CX_ROT_LENGTH        (FWD_LONG_CX_ROT_LENGTH<<1)
#define LONG_WINDOW_LENGTH                  1024
#define LONG_WINDOW_LENGTH_m_1              (LONG_WINDOW_LENGTH - 1)
#define TWICE_LONG_WINDOW_LENGTH_m_1        ((LONG_WINDOW_LENGTH<<1) - 1)

Int fwd_long_complex_rot(
    Int32 *Data_in,
    Int32 *Data_out,
    Int32  max);

#endif
