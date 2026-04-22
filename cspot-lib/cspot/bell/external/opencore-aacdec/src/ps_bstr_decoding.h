

#ifndef PS_BSTR_DECODING_H
#define PS_BSTR_DECODING_H

#include "pv_audio_type_defs.h"
#include "s_ps_dec.h"

extern const Int32 aNoIidBins[3];
extern const Int32 aNoIccBins[3];

extern const Int32 aFixNoEnvDecode[4];

#ifdef __cplusplus
extern "C"
{
#endif

    void ps_bstr_decoding(STRUCT_PS_DEC *h_ps_dec);

#ifdef __cplusplus
}
#endif

#endif
