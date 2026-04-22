

#ifndef ESC_IQUANT_SCALING_H
#define ESC_IQUANT_SCALING_H

#include "pv_audio_type_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void esc_iquant_scaling(
        const Int16   quantSpec[],
        Int32       coef[],
        const Int   sfbWidth,
        Int  const pQFormat,
        UInt16      scale,
        Int           maxInput);

#ifdef __cplusplus
}
#endif

#endif

