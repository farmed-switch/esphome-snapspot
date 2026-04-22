

#include "pv_audio_type_defs.h"

#include "e_rawbitstreamconst.h"
#include "e_tmp4audioobjecttype.h"

#include "s_bits.h"
#include "s_frameinfo.h"
#include "s_lt_pred_status.h"

#include "ibstream.h"
#include "lt_decode.h"
#include "ltp_common_internal.h"

#include "get_ics_info.h"
#include "huffman.h"

#define LEN_PREDICTOR_DATA_PRESENT (1)

Int get_ics_info(
    const tMP4AudioObjectType  audioObjectType,
    BITS                      *pInputStream,
    const Bool                 common_window,
    WINDOW_SEQUENCE           *pWindowSequence,
    WINDOW_SHAPE              *pWindowShape,
    Int                        group[],
    Int                       *p_max_sfb,
    FrameInfo                 *p_winmap[],
    LT_PRED_STATUS            *pFirstLTPStatus,
    LT_PRED_STATUS            *pSecondLTPStatus)
{
    WINDOW_SEQUENCE       windowSequence;
    UInt                  temp;
    Bool                  predictor_data_present;
    UInt                   local_max_sfb;
    UInt                   allowed_max_sfb;
    Int                   status = SUCCESS;
    Bool                  first_ltp_data_present = FALSE;
    Bool                  second_ltp_data_present = FALSE;

    temp =
        get9_n_lessbits(
            LEN_ICS_RESERV + LEN_WIN_SEQ + LEN_WIN_SH,
            pInputStream);

    windowSequence = (WINDOW_SEQUENCE)((temp >> LEN_WIN_SH) & ((0x1 << LEN_WIN_SEQ) - 1));

    *pWindowShape = (WINDOW_SHAPE)((temp) & ((0x1 << LEN_WIN_SH) - 1));

    allowed_max_sfb = p_winmap[windowSequence]->sfb_per_win[0];

    if (windowSequence == EIGHT_SHORT_SEQUENCE)
    {
        local_max_sfb =  get9_n_lessbits(LEN_MAX_SFBS,
                                         pInputStream);

        getgroup(
            group,
            pInputStream);

        if (local_max_sfb > allowed_max_sfb)
        {
            status = 1;
        }

    }
    else
    {

        group[0] = 1;

        temp =
            get9_n_lessbits(
                LEN_MAX_SFBL + LEN_PREDICTOR_DATA_PRESENT,
                pInputStream);

        local_max_sfb = (Int)(temp >> LEN_PREDICTOR_DATA_PRESENT);

        predictor_data_present =
            (Bool)(temp & ((0x1 << LEN_PREDICTOR_DATA_PRESENT) - 1));

        if (local_max_sfb > allowed_max_sfb)
        {
            status = 1;
        }
        else if (audioObjectType == MP4AUDIO_LTP)
        {

            if (predictor_data_present != FALSE)
            {
                first_ltp_data_present =
                    (Bool) get1bits(
                        pInputStream);

                if (first_ltp_data_present != FALSE)
                {
                    lt_decode(
                        windowSequence,
                        pInputStream,
                        local_max_sfb,
                        pFirstLTPStatus);
                }
                if (common_window != FALSE)
                {
                    second_ltp_data_present =
                        (Bool) get1bits(
                            pInputStream);

                    if (second_ltp_data_present != FALSE)
                    {
                        lt_decode(
                            windowSequence,
                            pInputStream,
                            local_max_sfb,
                            pSecondLTPStatus);
                    }
                }

            }

        }
        else
        {

            if (predictor_data_present != FALSE)
            {
                status = 1;
            }

        }

    }

    pFirstLTPStatus->ltp_data_present = first_ltp_data_present;
    if (common_window != FALSE)
    {
        pSecondLTPStatus->ltp_data_present = second_ltp_data_present;
    }

    *p_max_sfb = local_max_sfb;

    *pWindowSequence = windowSequence;

    return (status);

}

