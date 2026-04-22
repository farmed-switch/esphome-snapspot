

#ifndef PVMP4AUDIODECODER_API_H
#define PVMP4AUDIODECODER_API_H

#include "pv_audio_type_defs.h"

#include "e_tmp4audioobjecttype.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define PVMP4AUDIODECODER_INBUFSIZE  1536

    typedef enum ePVMP4AudioDecoderOutputFormat
    {
        OUTPUTFORMAT_16PCM_GROUPED = 0,
        OUTPUTFORMAT_16PCM_INTERLEAVED = 1

    } tPVMP4AudioDecoderOutputFormat;

    typedef enum ePVMP4AudioDecoderErrorCode
    {
        MP4AUDEC_SUCCESS           =  0,
        MP4AUDEC_INVALID_FRAME     = 10,
        MP4AUDEC_INCOMPLETE_FRAME  = 20,
        MP4AUDEC_LOST_FRAME_SYNC   = 30
    } tPVMP4AudioDecoderErrorCode;

    typedef enum
    {
        AAC = 0,
        AACPLUS,
        ENH_AACPLUS
    } STREAMTYPE;

    typedef struct
#ifdef __cplusplus
                tPVMP4AudioDecoderExternal
#endif
    {

        UChar  *pInputBuffer;

        Int     inputBufferCurrentLength;

        Int     inputBufferMaxLength;

        tPVMP4AudioDecoderOutputFormat  outputFormat;

        Int16  *pOutputBuffer;

        Int16  *pOutputBuffer_plus;

        Int32  aacPlusUpsamplingFactor;

        Bool    aacPlusEnabled;

        Bool    repositionFlag;

        Int     desiredChannels;

        Int     inputBufferUsedLength;

        Int32    remainderBits;

        Int32   samplingRate;

        Int32   bitRate;

        Int     encodedChannels;

        Int     frameLength;

        Int     audioObjectType;

        Int     extendedAudioObjectType;

    } tPVMP4AudioDecoderExternal;

    OSCL_IMPORT_REF UInt32 PVMP4AudioDecoderGetMemRequirements(void);

    OSCL_IMPORT_REF Int PVMP4AudioDecoderInitLibrary(
        tPVMP4AudioDecoderExternal  *pExt,
        void                        *pMem);

    OSCL_IMPORT_REF Int PVMP4AudioDecodeFrame(
        tPVMP4AudioDecoderExternal  *pExt,
        void                        *pMem);

    OSCL_IMPORT_REF Int PVMP4AudioDecoderConfig(
        tPVMP4AudioDecoderExternal  *pExt,
        void                        *pMem);

    OSCL_IMPORT_REF void PVMP4AudioDecoderResetBuffer(
        void                        *pMem);

    OSCL_IMPORT_REF void PVMP4AudioDecoderDisableAacPlus(
        tPVMP4AudioDecoderExternal  *pExt,
        void                        *pMem);

    Int PVMP4SetAudioConfig(
        tPVMP4AudioDecoderExternal  *pExt,
        void                        *pMem,
        Int                         upsamplingFactor,
        Int                         samp_rate,
        int                         num_ch,
        tMP4AudioObjectType         audioObjectType);

#ifdef __cplusplus
}
#endif

#endif

