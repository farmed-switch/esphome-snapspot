

#ifndef SBR_ENVELOPE_CALC_TBL_H
#define SBR_ENVELOPE_CALC_TBL_H

#include "pv_audio_type_defs.h"
#include "config.h"

extern const Int32 limGains[5];

extern const Int32 smoothLengths[2];

extern const Int16 rP_LCx[512];

#ifdef HQ_SBR

extern const Int32 fir_table[5][5];

extern const Int32 rPxx[512];

#endif

#endif

