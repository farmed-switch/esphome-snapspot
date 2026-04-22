

#include    "pv_audio_type_defs.h"
#include    "aac_mem_funcs.h"
#include    "s_frameinfo.h"
#include    "s_bits.h"
#include    "s_sectinfo.h"
#include    "s_huffman.h"
#include    "ibstream.h"

#include    "hcbtables.h"
#include    "e_huffmanconst.h"
#include    "e_infoinitconst.h"
#include    "huffman.h"

Int hufffac(
    FrameInfo   *pFrameInfo,
    BITS        *pInputStream,
    Int         *pGroup,
    Int         nsect,
    SectInfo    *pSect,
    Int         global_gain,
    Int         *pFactors,
    Int         huffBookUsed[])
{
    Int     sect_idx;
    Int     group_end;
    Int     group_win;
    Int     cw_index;
    Int     nsfb_win;
    Int     sfb;
    Int     sect_cb;
    Int     fac;
    Int     is_pos;
    Int     noise_pcm_flag = TRUE;
    Int     dpcm_noise_nrg;
    Int     noise_nrg;
    Int     status = SUCCESS;
    Int     *pHuffBookUsed = &huffBookUsed[0];

    pv_memset(pFactors,
              ZERO_HCB,
              MAXBANDS*sizeof(*pFactors));

    if (nsect)
    {

        if (nsect == 1)
        {
            sect_cb  = pSect->sect_cb;

            for (sfb = pSect->sect_end >> 2; sfb != 0; sfb--)
            {
                *(pHuffBookUsed++) = sect_cb;
                *(pHuffBookUsed++) = sect_cb;
                *(pHuffBookUsed++) = sect_cb;
                *(pHuffBookUsed++) = sect_cb;
            }
            for (sfb = pSect->sect_end & 3; sfb != 0; sfb--)
            {
                *(pHuffBookUsed++) = sect_cb;
            }

        }
        else
        {
            Int sect_start = 0;
            for (sect_idx = nsect; sect_idx > 0; sect_idx--)
            {
                sect_cb  = pSect->sect_cb;

                for (sfb = sect_start; sfb < pSect->sect_end; sfb++)
                {
                    pHuffBookUsed[sfb] = sect_cb;
                }

                pSect++;
                sect_start = sfb;

            }
        }
    }
    else
    {

        pv_memset(pHuffBookUsed,
                  ZERO_HCB,
                  MAXBANDS*sizeof(*pHuffBookUsed));
    }

    pHuffBookUsed = &huffBookUsed[0];

    fac       = global_gain;
    is_pos    = 0;
    noise_nrg = global_gain - NOISE_OFFSET;

    group_win  = 0;
    group_end  = 0;

    while ((group_end < pFrameInfo->num_win) && (status == SUCCESS))
    {
        nsfb_win  = pFrameInfo->sfb_per_win[group_end];
        group_end = *pGroup++;

        for (sfb = 0; sfb < nsfb_win; sfb++)
        {

            switch (pHuffBookUsed[sfb])
            {
                case ZERO_HCB:
                    break;
                case INTENSITY_HCB:
                case INTENSITY_HCB2:

                    cw_index = decode_huff_scl(pInputStream);

                    is_pos        += cw_index - MIDFAC;
                    pFactors[sfb] =  is_pos;
                    break;
                case NOISE_HCB:

                    if (noise_pcm_flag == TRUE)
                    {
                        noise_pcm_flag = FALSE;
                        dpcm_noise_nrg = get9_n_lessbits(NOISE_PCM_BITS,
                                                         pInputStream);

                        dpcm_noise_nrg -= NOISE_PCM_OFFSET;
                    }
                    else
                    {
                        dpcm_noise_nrg = decode_huff_scl(pInputStream);

                        dpcm_noise_nrg -= MIDFAC;
                    }

                    noise_nrg       += dpcm_noise_nrg;
                    pFactors[sfb]   =  noise_nrg;
                    break;
                case BOOKSCL:
                    status = 1;
                    sfb = nsfb_win;
                    break;
                default:

                    cw_index = decode_huff_scl(pInputStream);

                    fac      += cw_index - MIDFAC;
                    if ((fac >= 2*TEXP) || (fac < 0))
                    {
                        status = 1;
                    }
                    else
                    {
                        pFactors[sfb] = fac;
                    }
            }

        }

        if (pFrameInfo->islong == FALSE)
        {

            for (group_win++; group_win < group_end; group_win++)
            {
                for (sfb = 0; sfb < nsfb_win; sfb++)
                {
                    pFactors[sfb + nsfb_win]  =  pFactors[sfb];
                }
                pFactors  +=  nsfb_win;
            }

        }

        pHuffBookUsed   += nsfb_win;
        pFactors        += nsfb_win;

    }

    return status;

}

