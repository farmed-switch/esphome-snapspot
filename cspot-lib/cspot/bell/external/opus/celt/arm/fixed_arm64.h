

#ifndef FIXED_ARM64_H
#define FIXED_ARM64_H

#include <arm_neon.h>

#undef SIG2WORD16
#define SIG2WORD16(x) (vqmovns_s32(PSHR32((x), SIG_SHIFT)))

#endif
