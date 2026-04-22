

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_get_dir_control_data.h"
#include    "buf_getbits.h"

void sbr_get_dir_control_data(SBR_FRAME_DATA * h_frame_data,
                              BIT_BUFFER     * hBitBuf)
{
    Int32 i;

    h_frame_data->nNoiseFloorEnvelopes = h_frame_data->frameInfo[0] > 1 ? 2 : 1;

    for (i = 0; i < h_frame_data->frameInfo[0]; i++)
    {
        h_frame_data->domain_vec1[i] = buf_getbits(hBitBuf, SI_SBR_DOMAIN_BITS);
    }

    for (i = 0; i < h_frame_data->nNoiseFloorEnvelopes; i++)
    {
        h_frame_data->domain_vec2[i] = buf_getbits(hBitBuf, SI_SBR_DOMAIN_BITS);
    }
}

#endif

