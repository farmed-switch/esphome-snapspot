

#ifndef PS_ALL_PASS_FILTER_COEFF_H
#define PS_ALL_PASS_FILTER_COEFF_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

extern const Char groupBorders[NO_IID_GROUPS + 1];
extern const Int16 aRevLinkDecaySerCoeff[NO_ALLPASS_CHANNELS][NO_SERIAL_ALLPASS_LINKS];
extern const Int32  aRevLinkDelaySer[];
extern const Int16 aFractDelayPhaseFactorReQmf[NO_QMF_ALLPASS_CHANNELS];
extern const Int16 aFractDelayPhaseFactorImQmf[NO_QMF_ALLPASS_CHANNELS];

extern const Int16 aFractDelayPhaseFactorReSubQmf[SUBQMF_GROUPS];
extern const Int16 aFractDelayPhaseFactorImSubQmf[SUBQMF_GROUPS];
extern const Int32 aFractDelayPhaseFactorSubQmf[SUBQMF_GROUPS];
extern const Int32 aFractDelayPhaseFactor[NO_QMF_ALLPASS_CHANNELS];
extern const Int32 aaFractDelayPhaseFactorSerQmf[NO_QMF_ALLPASS_CHANNELS][3];
extern const Int32 aaFractDelayPhaseFactorSerSubQmf[SUBQMF_GROUPS][3];
extern const Char bins2groupMap[NO_IID_GROUPS];
extern const Int32 aaFractDelayPhaseFactorSerReQmf[NO_QMF_ALLPASS_CHANNELS][3];
extern const Int32 aaFractDelayPhaseFactorSerImQmf[NO_QMF_ALLPASS_CHANNELS][3];
extern const Int32 aaFractDelayPhaseFactorSerReSubQmf[SUBQMF_GROUPS][3];
extern const Int32 aaFractDelayPhaseFactorSerImSubQmf[SUBQMF_GROUPS][3];

#endif
