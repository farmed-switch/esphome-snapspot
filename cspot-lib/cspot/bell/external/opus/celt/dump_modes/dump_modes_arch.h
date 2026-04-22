

#ifndef DUMP_MODE_ARCH_H
#define DUMP_MODE_ARCH_H

void dump_modes_arch_init();
void dump_mode_arch(CELTMode *mode);
void dump_modes_arch_finalize();

#if !defined(FIXED_POINT)
#define ARM_NE10_ARCH_FILE_NAME "static_modes_float_arm_ne10.h"
#else
#define ARM_NE10_ARCH_FILE_NAME "static_modes_fixed_arm_ne10.h"
#endif

#if defined(HAVE_ARM_NE10)
#define OVERRIDE_FFT (1)
#endif

#endif
