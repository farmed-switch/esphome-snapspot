

#ifndef SBR_UPDATE_FREQ_SCALE_H
#define SBR_UPDATE_FREQ_SCALE_H

#include "pv_audio_type_defs.h"

#define MAX_OCTAVE        29
#define MAX_SECOND_REGION 50

void sbr_update_freq_scale(Int32 * v_k_master,
                           Int32 *h_num_bands,
                           const Int32 lsbM,
                           const Int32 usb,
                           const Int32 freqScale,
                           const Int32 alterScale,
                           const Int32 channelOffset);

void CalcBands(Int32 * diff,
               Int32 start,
               Int32 stop,
               Int32 num_bands);

void cumSum(Int32 start_value,
            Int32 * diff,
            Int32 length,
            Int32 * start_adress);

#endif

