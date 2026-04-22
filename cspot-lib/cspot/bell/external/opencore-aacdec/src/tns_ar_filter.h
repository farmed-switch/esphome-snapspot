

#ifndef TNS_AR_FILTER_H
#define TNS_AR_FILTER_H

#include "e_tns_const.h"

#ifdef __cplusplus
extern "C"
{
#endif

    Int tns_ar_filter(
        Int32 spec[],
        const Int spec_length,
        const Int inc,
        const Int32 lpc[],
        const Int lpc_qformat,
        const Int order);

#ifdef __cplusplus
}
#endif

#endif
