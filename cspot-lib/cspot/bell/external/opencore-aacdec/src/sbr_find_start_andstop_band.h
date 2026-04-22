

#ifndef SBR_FIND_START_ANDSTOP_BAND_H
#define SBR_FIND_START_ANDSTOP_BAND_H

#include "pv_audio_type_defs.h"
#include "e_sbr_error.h"

SBR_ERROR sbr_find_start_andstop_band(const Int32 samplingFreq,
                                      const Int32 startFreq,
                                      const Int32 stopFreq,
                                      Int *lsbM,
                                      Int *usb);

#endif

