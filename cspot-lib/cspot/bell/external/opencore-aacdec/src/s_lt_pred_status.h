

#ifndef S_LT_PRED_STATUS_H
#define S_LT_PRED_STATUS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "e_blockswitching.h"

#ifndef MAX_SHORT_WINDOWS
#define MAX_SHORT_WINDOWS NSHORT
#endif

#ifndef MAX_SCFAC_BANDS
#define MAX_SCFAC_BANDS MAXBANDS
#endif

#ifndef BLOCK_LEN_LONG
#define BLOCK_LEN_LONG LN2
#endif

#define LTP_MAX_BLOCK_LEN_LONG BLOCK_LEN_LONG

#ifndef LT_BLEN
#define LT_BLEN (2 * LTP_MAX_BLOCK_LEN_LONG)
#endif

    typedef struct
    {
        Int weight_index;
        Int win_prediction_used[MAX_SHORT_WINDOWS];
        Int sfb_prediction_used[MAX_SCFAC_BANDS];
        Bool ltp_data_present;

        Int delay[MAX_SHORT_WINDOWS];
    }
    LT_PRED_STATUS;

#ifdef __cplusplus
}
#endif

#endif

