

#ifndef SBR_OPEN_H
#define SBR_OPEN_H

#include "s_sbr_channel.h"
#include "sbr_dec.h"

void sbr_open(Int32 sampleRate,
              SBR_DEC *sbrDec,
              SBRDECODER_DATA * self,
              Bool bDownSampledSbr);

#endif

