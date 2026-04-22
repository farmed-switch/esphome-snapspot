

#ifndef SILK_MACROS_ARM64_H
#define SILK_MACROS_ARM64_H

#include <arm_neon.h>

#undef silk_ADD_SAT32
#define silk_ADD_SAT32(a, b) (vqadds_s32((a), (b)))

#undef silk_SUB_SAT32
#define silk_SUB_SAT32(a, b) (vqsubs_s32((a), (b)))

#endif
