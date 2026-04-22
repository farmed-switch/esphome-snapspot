

#ifndef S_HYBRID_H
#define S_HYBRID_H

#include    "ps_constants.h"
#include    "pv_audio_type_defs.h"

#define HYBRID_FILTER_LENGTH        13
#define HYBRID_FILTER_LENGTH_m_1    12
#define HYBRID_FILTER_DELAY         6

typedef enum
{

    HYBRID_2_REAL = 2,
    HYBRID_4_CPLX = 4,
    HYBRID_8_CPLX = 8

} HYBRID_RES;

typedef struct
{
    Int32   nQmfBands;
    Int32   *pResolution;
    Int32   qmfBufferMove;

    Int32 **mQmfBufferReal;
    Int32 **mQmfBufferImag;
    Int32 *mTempReal;
    Int32 *mTempImag;

} HYBRID;

#endif
