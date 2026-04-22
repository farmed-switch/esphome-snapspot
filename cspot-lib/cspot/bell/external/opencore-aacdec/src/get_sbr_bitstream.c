

#include    "config.h"

#ifdef AAC_PLUS

#include "get_sbr_bitstream.h"
#include "pv_audio_type_defs.h"
#include    "sbr_crc_check.h"

void get_sbr_bitstream(SBRBITSTREAM *sbrBitStream, BITS *pInputStream)
{

    Int32 count;
    Int32 esc_count;
    Int32 Extention_Type;
    Int32 i;

    count = get9_n_lessbits(LEN_F_CNT, pInputStream);
    if (count == 15)
    {
        esc_count = get9_n_lessbits(LEN_F_ESC, pInputStream);
        count = esc_count + 14;
    }

    Extention_Type = get9_n_lessbits(LEN_F_CNT, pInputStream);

    if (((Extention_Type == SBR_EXTENSION) || (Extention_Type == SBR_EXTENSION_CRC))
            && (count < MAXSBRBYTES) && (count) && (sbrBitStream->NrElements < MAXNRELEMENTS))
    {

        sbrBitStream->sbrElement[sbrBitStream->NrElements].ExtensionType = Extention_Type;
        sbrBitStream->sbrElement[sbrBitStream->NrElements].Payload       = count;
        sbrBitStream->sbrElement[sbrBitStream->NrElements].Data[0]       = (UChar) get9_n_lessbits(LEN_F_CNT, pInputStream);
        for (i = 1 ; i < count ; i++)
        {
            sbrBitStream->sbrElement[sbrBitStream->NrElements].Data[i] = (UChar) get9_n_lessbits(8, pInputStream);
        }

        sbrBitStream->NrElements += 1;

    }
    else
    {
        pInputStream->usedBits += (count - 1) * LEN_BYTE;
        pInputStream->usedBits += 4;

    }
}

#endif
