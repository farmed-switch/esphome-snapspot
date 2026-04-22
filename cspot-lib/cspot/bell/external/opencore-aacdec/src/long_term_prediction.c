

#include "pv_audio_type_defs.h"
#include "e_window_sequence.h"
#include "ltp_common_internal.h"
#include "long_term_prediction.h"
#include "aac_mem_funcs.h"
#include "pv_normalize.h"
#include "window_block_fxp.h"

const UInt codebook[CODESIZE] =
{
    18705,
    22827,
    26641,
    29862,
    32273,
    34993,
    39145,
    44877
};

Int long_term_prediction(
    WINDOW_SEQUENCE     win_seq,
    const Int           weight_index,
    const Int           delay[],
    const Int16         buffer[],
    const Int           buffer_offset,
    const Int32         time_quant[],
    Int32         predicted_samples[],
    const Int           frame_length)
{

    const Int16 *pBuffer;

    const Int32 *pTimeQuant = time_quant;

    Int32 *pPredicted_samples;

    Int32   test;
    Int32   datum;

    UInt    weight;

    Int     block_length;

    Int     shift;
    Int     k;
    Int     ltp_buffer_index;
    Int     jump_point;
    Int     lag;
    Int     num_samples;

    Int32   max = 0;

    pPredicted_samples = &predicted_samples[0];

    weight = codebook[weight_index];

    if (win_seq != EIGHT_SHORT_SEQUENCE)
    {

        block_length = frame_length << 1;

        lag = delay[0];

        ltp_buffer_index = block_length - lag;

        if (lag < frame_length)
        {
            num_samples = frame_length + lag;
        }
        else
        {
            num_samples = block_length;
        }

        block_length -= num_samples;

        jump_point = (frame_length - ltp_buffer_index);

        if (jump_point > 0)
        {
            pBuffer = &(buffer[ltp_buffer_index + buffer_offset]);

            for (k = jump_point; k > 0; k--)
            {

                test = (Int32) weight * (*(pBuffer++));
                *(pPredicted_samples++) =  test;
                max                   |= (test >> 31) ^ test;
            }

            num_samples -= jump_point;

            ltp_buffer_index += jump_point;
        }

        jump_point = 2 * frame_length - ltp_buffer_index;

        pBuffer = &(buffer[ltp_buffer_index - buffer_offset]);

        if (num_samples < jump_point)
        {
            jump_point = num_samples;
        }

        for (k = jump_point; k > 0; k--)
        {

            test = (Int32) weight * (*(pBuffer++));
            *(pPredicted_samples++) =  test;
            max                   |= (test >> 31) ^ test;
        }

        num_samples -= jump_point;

        ltp_buffer_index += jump_point;

        for (k = num_samples; k > 0; k--)
        {

            datum = *(pTimeQuant++) >> SCALING;

            test                    = (Int32)datum * weight;
            *(pPredicted_samples++) =  test;
            max                    |= (test >> 31) ^ test;

        }

        pv_memset(
            pPredicted_samples,
            0,
            block_length*sizeof(*pPredicted_samples));

    }

    shift = 16 - pv_normalize(max);

    if (shift < 0)
    {
        shift = 0;
    }

    return (shift);
}

