

#ifndef CALC_AUTO_CORR_H
#define CALC_AUTO_CORR_H

#include "pv_audio_type_defs.h"
#include "config.h"

struct ACORR_COEFS
{
    Int32  r11r;
    Int32  r01r;
    Int32  r02r;
    Int32  r12r;
    Int32  r22r;
#ifdef HQ_SBR
    Int32  r01i;
    Int32  r02i;
    Int32  r12i;
#endif
    Int32  det;
};

#ifdef __cplusplus
extern "C"
{
#endif

    void calc_auto_corr_LC(struct ACORR_COEFS *ac,
    Int32  realBuf[][32],
    Int32  bd,
    Int32  len);

#ifdef HQ_SBR

    void calc_auto_corr(struct ACORR_COEFS *ac,
                        Int32  realBuf[][32],
                        Int32  imagBuf[][32],
                        Int32  bd,
                        Int32  len);

#endif

#ifdef __cplusplus
}
#endif

#endif

