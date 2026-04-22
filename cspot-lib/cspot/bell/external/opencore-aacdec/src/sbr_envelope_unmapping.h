

#ifndef SBR_ENVELOPE_UNMAPPING_H
#define SBR_ENVELOPE_UNMAPPING_H

#include    "pv_audio_type_defs.h"
#include    "s_sbr_frame_data.h"

#define UNMAPPING_SCALE_INT         (-18)
#define SBR_ENERGY_PAN_OFFSET_INT   12

void sbr_envelope_unmapping(SBR_FRAME_DATA * hFrameData1,
                            SBR_FRAME_DATA * hFrameData2);

#endif

