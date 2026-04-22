

#ifndef S_TNS_FRAME_INFO_H
#define S_TNS_FRAME_INFO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_tns_const.h"
#include "s_tnsfilt.h"

    typedef struct
    {
        Bool tns_data_present;

        Int n_filt[TNS_MAX_WIN];

        TNSfilt filt[TNS_MAX_WIN];

        Int32 lpc_coef[3*TNS_MAX_ORDER];

    } TNS_frame_info;

#ifdef __cplusplus
}
#endif

#endif
