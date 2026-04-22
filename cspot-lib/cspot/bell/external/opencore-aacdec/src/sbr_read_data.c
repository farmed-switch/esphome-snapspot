

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_read_data.h"
#include    "s_bit_buffer.h"
#include    "buf_getbits.h"
#include    "sbr_get_sce.h"
#include    "sbr_get_cpe.h"
#include    "sbr_reset_dec.h"
#include    "sbr_get_header_data.h"
#include    "sbr_crc_check.h"
#include    "aac_mem_funcs.h"

#include    "init_sbr_dec.h"

SBR_ERROR sbr_read_data(SBRDECODER_DATA * self,
                        SBR_DEC * sbrDec,
                        SBRBITSTREAM *stream)
{
    SBR_ERROR sbr_err =  SBRDEC_OK;
    Int32 SbrFrameOK = 1;
    Int32 sbrCRCAlwaysOn = 0;

    UInt32 bs_header_flag = 0;

    SBR_HEADER_STATUS headerStatus = HEADER_OK;

    SBR_CHANNEL *SbrChannel = self->SbrChannel;

    Int32 zeropadding_bits;
    Int32 lr;

    BIT_BUFFER bitBuf ;

    bitBuf.buffer_word    = 0;
    bitBuf.buffered_bits  = 0;
    bitBuf.nrBitsRead     = 0;

    bitBuf.char_ptr  =  stream->sbrElement[0].Data;
    bitBuf.bufferLen = (stream->sbrElement[0].Payload) << 3;

    buf_getbits(&bitBuf, LEN_NIBBLE);

    if ((stream->sbrElement[0].ExtensionType == SBR_EXTENSION_CRC) ||
            sbrCRCAlwaysOn)
    {
        Int32 CRCLen = ((stream->sbrElement[0].Payload - 1) << 3) + 4 - SI_SBR_CRC_BITS;
        SbrFrameOK = sbr_crc_check(&bitBuf, CRCLen);
    }

    if (SbrFrameOK)
    {

        bs_header_flag = buf_getbits(&bitBuf, 1);

        if (bs_header_flag)
        {

            headerStatus = sbr_get_header_data(&(SbrChannel[0].frameData.sbr_header),
                                               &bitBuf,
                                               SbrChannel[0].syncState);
        }

        switch (stream->sbrElement[0].ElementID)
        {
            case SBR_ID_SCE :

                if (headerStatus == HEADER_RESET)
                {
                    sbr_err = sbr_reset_dec(&(SbrChannel[0].frameData),
                                            sbrDec,
                                            self->SbrChannel[0].frameData.sbr_header.sampleRateMode);

                    if (sbr_err != SBRDEC_OK)
                    {
                        break;
                    }

                    SbrChannel[0].syncState     = SBR_ACTIVE;
                }

                if ((SbrChannel[0].syncState == SBR_ACTIVE))
                {
                    sbr_err = sbr_get_sce(&(SbrChannel[0].frameData),
                                          &bitBuf
#ifdef PARAMETRICSTEREO
                                          , self->hParametricStereoDec
#endif
                                         );

                    if (sbr_err != SBRDEC_OK)
                    {
                        break;
                    }
                }

                break;

            case SBR_ID_CPE :

                if (bs_header_flag)
                {
                    pv_memcpy(&(SbrChannel[1].frameData.sbr_header),
                              &(SbrChannel[0].frameData.sbr_header),
                              sizeof(SBR_HEADER_DATA));
                }

                if (headerStatus == HEADER_RESET)
                {
                    for (lr = 0 ; lr < 2 ; lr++)
                    {
                        sbr_err = sbr_reset_dec(&(SbrChannel[lr].frameData),
                                                sbrDec,
                                                self->SbrChannel[0].frameData.sbr_header.sampleRateMode);

                        if (sbr_err != SBRDEC_OK)
                        {
                            break;
                        }

                        SbrChannel[lr].syncState = SBR_ACTIVE;
                    }
                }

                if (SbrChannel[0].syncState == SBR_ACTIVE)
                {
                    sbr_err = sbr_get_cpe(&(SbrChannel[0].frameData),
                                          &(SbrChannel[1].frameData),
                                          &bitBuf);

                    if (sbr_err != SBRDEC_OK)
                    {
                        break;
                    }

                }
                break;

            default:
                sbr_err = SBRDEC_ILLEGAL_PLUS_ELE_ID;
                break;
        }

    }

    zeropadding_bits = (8 - (bitBuf.nrBitsRead & 0x7)) & 0x7;

    if ((bitBuf.nrBitsRead + zeropadding_bits)  > bitBuf.bufferLen)
    {
        sbr_err = SBRDEC_INVALID_BITSTREAM;
    }

    return sbr_err;
}

#endif

