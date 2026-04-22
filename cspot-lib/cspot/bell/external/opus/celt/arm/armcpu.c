

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef OPUS_HAVE_RTCD

#include "armcpu.h"
#include "cpu_support.h"
#include "os_support.h"
#include "opus_types.h"
#include "arch.h"

#define OPUS_CPU_ARM_V4_FLAG    (1<<OPUS_ARCH_ARM_V4)
#define OPUS_CPU_ARM_EDSP_FLAG  (1<<OPUS_ARCH_ARM_EDSP)
#define OPUS_CPU_ARM_MEDIA_FLAG (1<<OPUS_ARCH_ARM_MEDIA)
#define OPUS_CPU_ARM_NEON_FLAG  (1<<OPUS_ARCH_ARM_NEON)

#if defined(_MSC_VER)

# define WIN32_LEAN_AND_MEAN
# define WIN32_EXTRA_LEAN
# include <windows.h>

static OPUS_INLINE opus_uint32 opus_cpu_capabilities(void){
  opus_uint32 flags;
  flags=0;

# if defined(OPUS_ARM_MAY_HAVE_EDSP) || defined(OPUS_ARM_MAY_HAVE_MEDIA) \
 || defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
  __try{

    __emit(0xF5DDF000);
    flags|=OPUS_CPU_ARM_EDSP_FLAG;
  }
  __except(GetExceptionCode()==EXCEPTION_ILLEGAL_INSTRUCTION){

  }
#  if defined(OPUS_ARM_MAY_HAVE_MEDIA) \
 || defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
  __try{

    __emit(0xE6333F93);
    flags|=OPUS_CPU_ARM_MEDIA_FLAG;
  }
  __except(GetExceptionCode()==EXCEPTION_ILLEGAL_INSTRUCTION){

  }
#   if defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
  __try{

    __emit(0xF2200150);
    flags|=OPUS_CPU_ARM_NEON_FLAG;
  }
  __except(GetExceptionCode()==EXCEPTION_ILLEGAL_INSTRUCTION){

  }
#   endif
#  endif
# endif
  return flags;
}

#elif defined(__linux__)

#include <stdio.h>

opus_uint32 opus_cpu_capabilities(void)
{
  opus_uint32 flags = 0;
  FILE *cpuinfo;

  cpuinfo = fopen("/proc/cpuinfo", "r");

  if(cpuinfo != NULL)
  {

    char buf[512];

    while(fgets(buf, 512, cpuinfo) != NULL)
    {
# if defined(OPUS_ARM_MAY_HAVE_EDSP) || defined(OPUS_ARM_MAY_HAVE_MEDIA) \
 || defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)

      if(memcmp(buf, "Features", 8) == 0)
      {
        char *p;
        p = strstr(buf, " edsp");
        if(p != NULL && (p[5] == ' ' || p[5] == '\n'))
          flags |= OPUS_CPU_ARM_EDSP_FLAG;

#  if defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)
        p = strstr(buf, " neon");
        if(p != NULL && (p[5] == ' ' || p[5] == '\n'))
          flags |= OPUS_CPU_ARM_NEON_FLAG;
#  endif
      }
# endif

# if defined(OPUS_ARM_MAY_HAVE_MEDIA) \
 || defined(OPUS_ARM_MAY_HAVE_NEON) || defined(OPUS_ARM_MAY_HAVE_NEON_INTR)

      if(memcmp(buf, "CPU architecture:", 17) == 0)
      {
        int version;
        version = atoi(buf+17);

        if(version >= 6)
          flags |= OPUS_CPU_ARM_MEDIA_FLAG;
      }
# endif
    }

    fclose(cpuinfo);
  }
  return flags;
}
#else

# error "Configured to use ARM asm but no CPU detection method available for " \
   "your platform.  Reconfigure with --disable-rtcd (or send patches)."
#endif

int opus_select_arch(void)
{
  opus_uint32 flags = opus_cpu_capabilities();
  int arch = 0;

  if(!(flags & OPUS_CPU_ARM_EDSP_FLAG)) {

    celt_assert(arch == OPUS_ARCH_ARM_V4);
    return arch;
  }
  arch++;

  if(!(flags & OPUS_CPU_ARM_MEDIA_FLAG)) {
    celt_assert(arch == OPUS_ARCH_ARM_EDSP);
    return arch;
  }
  arch++;

  if(!(flags & OPUS_CPU_ARM_NEON_FLAG)) {
    celt_assert(arch == OPUS_ARCH_ARM_MEDIA);
    return arch;
  }
  arch++;

  celt_assert(arch == OPUS_ARCH_ARM_NEON);
  return arch;
}

#endif
