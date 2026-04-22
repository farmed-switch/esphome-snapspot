

#ifndef S_TDEC_INT_FILE_H
#define S_TDEC_INT_FILE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "pv_audio_type_defs.h"
#include "s_progconfig.h"
#include "s_frameinfo.h"
#include "s_mc_info.h"
#include "s_adif_header.h"
#include "s_tdec_int_chan.h"
#include "s_pulseinfo.h"
#include "s_bits.h"
#include "s_hcb.h"
#include "e_infoinitconst.h"

#include "s_sbr_channel.h"
#include "s_sbr_dec.h"
#include "s_sbrbitstream.h"
#include "config.h"

    typedef struct
    {
        UInt32         bno;
        Int            status;

        Bool           aacPlusEnabled;
        Bool           aacConfigUtilityEnabled;

        Int            current_program;
        Int            frameLength;
        Int            adif_test;

        BITS           inputStream;

        ProgConfig     prog_config;

        Int            SFBWidth128[(1<<LEN_MAX_SFBS)];

        FrameInfo      longFrameInfo;
        FrameInfo      shortFrameInfo;
        FrameInfo     *winmap[NUM_WIN_SEQ];

        Int32          pns_cur_noise_state;

        MC_Info        mc_info;

        Int            ltp_buffer_state;

        tDec_Int_Chan  perChan[Chans];

        Int32          fxpCoef[2][LN];

#ifdef AAC_PLUS

        SBRDECODER_DATA sbrDecoderData;
        SBR_DEC         sbrDec;
        SBRBITSTREAM    sbrBitStr;

#endif

        UInt32         syncword;
        Int            invoke;

        Int         mask[MAXBANDS];
        Int         hasmask;

        union scratch_memory
        {
            Int32  fft[LONG_WINDOW];
            Int32  tns_inv_filter[TNS_MAX_ORDER];
            Int32  tns_decode_coef[2*TNS_MAX_ORDER];
            Int    huffbook_used[248];
            Int16  tmp_spec[LN2];

            ADIF_Header    adif_header;

            ProgConfig     scratch_prog_config;

            Int32  scratch_mem[16][64];
        } scratch;

        union shared_memory
        {
            Int32       predictedSamples[LONG_BLOCK1];

            Char        data_stream_bytes[(1<<LEN_D_CNT)+1];

            struct
            {
                Int16         quantSpec[LN2];
                SectInfo    sect[MAXBANDS + 1];
                PulseInfo   pulseInfo;
            } a;

        } share;

    } tDec_Int_File;

#ifdef __cplusplus
}
#endif

#endif
