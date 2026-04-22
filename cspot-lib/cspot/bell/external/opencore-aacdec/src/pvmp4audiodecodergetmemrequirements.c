

#include "pv_audio_type_defs.h"
#include "s_tdec_int_file.h"
#include "pvmp4audiodecoder_api.h"

OSCL_EXPORT_REF UInt32 PVMP4AudioDecoderGetMemRequirements(void)
{
    UInt32 size;

    size = (UInt32) sizeof(tDec_Int_File);

    return (size);

}

