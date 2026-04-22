

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_downsample_lo_res.h"
#include    "sbr_constants.h"

void  sbr_downsample_lo_res(Int32 v_result[],
                            Int32 num_result,
                            Int   freqBandTableRef[],
                            Int32 num_Ref)
{
    Int32 step;
    Int32 i, j;
    Int32 org_length;
    Int32 result_length;
    Int32 v_index[MAX_FREQ_COEFFS/2];

    org_length = num_Ref;
    result_length = num_result;

    v_index[0] = 0;
    i = 0;
    while (org_length > 0)
    {
        i++;
        step = org_length / result_length;
        org_length = org_length - step;
        result_length--;
        v_index[i] = v_index[i-1] + step;
    }

    for (j = 0; j <= i; j++)
    {
        v_result[j] = freqBandTableRef[ v_index[j]];
    }

}

#endif

