

#ifndef FXP_MUL32
#define FXP_MUL32

#include "config.h"

#if   defined(PV_ARM_V5)

#include "fxp_mul32_arm_v5.h"

#elif defined(PV_ARM_V4)

#include "fxp_mul32_arm_v4.h"

#elif defined(PV_ARM_MSC_EVC_V4)

#include "fxp_mul32_c_msc_evc.h"

#elif defined(PV_ARM_MSC_EVC_V5)

#include "fxp_mul32_c_msc_evc_armv5.h"

#elif defined(PV_ARM_GCC_V5)

#include "fxp_mul32_arm_gcc.h"

#elif defined(PV_ARM_GCC_V4)

#include "fxp_mul32_arm_v4_gcc.h"

#else

#ifndef C_EQUIVALENT
#define C_EQUIVALENT
#endif

#include "fxp_mul32_c_equivalent.h"

#endif

#endif

