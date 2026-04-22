

#ifndef S_PULSEINFO_H
#define S_PULSEINFO_H

#include "pv_audio_type_defs.h"
#include "e_rawbitstreamconst.h"

typedef struct
{
    Int pulse_data_present;
    Int number_pulse;
    Int pulse_start_sfb;
    Int pulse_offset[NUM_PULSE_LINES];
    Int pulse_amp[NUM_PULSE_LINES];
} PulseInfo;

#endif

