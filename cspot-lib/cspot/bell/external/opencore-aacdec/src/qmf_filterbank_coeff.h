

#ifndef QMF_FILTERBANK_COEFF_H
#define QMF_FILTERBANK_COEFF_H

#include "pv_audio_type_defs.h"
#include "config.h"

#define Qfmt(x)   (Int16)(x*(((Int32)1<<15)*1.11111111111111111F) + (x>=0?0.5F:-0.5F))

#define Qfmt30(x)   (Int32)(x*((Int32)1<<30) + (x>=0?0.5F:-0.5F))
#define Qfmt27(x)   (Int32)(x*(((Int32)1<<27)) + (x>=0?0.5F:-0.5F))

extern const Int32 sbrDecoderFilterbankCoefficients[155];

extern const Int32 sbrDecoderFilterbankCoefficients_down_smpl[160];
extern const Int32 sbrDecoderFilterbankCoefficients_an_filt_LC[155];

#ifdef HQ_SBR
extern const Int32 sbrDecoderFilterbankCoefficients_an_filt[155];
#endif

#endif

