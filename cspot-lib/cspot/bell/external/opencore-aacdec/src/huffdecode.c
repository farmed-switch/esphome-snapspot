

#include    "pv_audio_type_defs.h"
#include    "aac_mem_funcs.h"
#include    "huffman.h"
#include    "e_maskstatus.h"
#include    "e_elementid.h"
#include    "get_ics_info.h"

#define LEFT  (0)
#define RIGHT (1)

Int huffdecode(
    Int           id_syn_ele,
    BITS          *pInputStream,
    tDec_Int_File *pVars,
    tDec_Int_Chan *pChVars[])

{

    Int      ch;
    Int      common_window;
    Int      hasmask;
    Int      status   = SUCCESS;
    Int      num_channels = 0;
    MC_Info  *pMcInfo;

    per_chan_share_w_fxpCoef *pChLeftShare;
    per_chan_share_w_fxpCoef *pChRightShare;

    get9_n_lessbits(
        LEN_TAG,
        pInputStream);

    common_window = 0;

    if (id_syn_ele == ID_CPE)
    {
        common_window =
            get1bits(pInputStream);
    }

    pMcInfo = &pVars->mc_info;

    if ((pMcInfo->ch_info[0].cpe != id_syn_ele))
    {
        if (pVars->mc_info.implicit_channeling)
        {
            pMcInfo->ch_info[0].cpe = id_syn_ele & 1;
            pMcInfo->nch = (id_syn_ele & 1) + 1;
        }
        else
        {
            status = 1;
        }
    }

    if (status == SUCCESS)
    {
        if (id_syn_ele == ID_SCE)
        {

            num_channels = 1;
            pVars->hasmask = 0;
        }
        else if (id_syn_ele == ID_CPE)
        {
            pChLeftShare = pChVars[LEFT]->pShareWfxpCoef;
            pChRightShare = pChVars[RIGHT]->pShareWfxpCoef;
            num_channels = 2;

            if (common_window != FALSE)
            {

                status = get_ics_info(
                             (tMP4AudioObjectType) pVars->mc_info.audioObjectType,
                             pInputStream,
                             (Bool)common_window,
                             (WINDOW_SEQUENCE *) & pChVars[LEFT]->wnd,
                             (WINDOW_SHAPE *) & pChVars[LEFT]->wnd_shape_this_bk,
                             pChLeftShare->group,
                             (Int *) & pChLeftShare->max_sfb,
                             pVars->winmap,
                             (LT_PRED_STATUS *) & pChLeftShare->lt_status,
                             (LT_PRED_STATUS *) & pChRightShare->lt_status);

                if (status == SUCCESS)
                {

                    pChVars[RIGHT]->wnd = pChVars[LEFT]->wnd;
                    pChVars[RIGHT]->wnd_shape_this_bk =
                        pChVars[LEFT]->wnd_shape_this_bk;
                    pChRightShare->max_sfb = pChLeftShare->max_sfb;
                    pv_memcpy(
                        pChRightShare->group,
                        pChLeftShare->group,
                        NSHORT*sizeof(pChLeftShare->group[0]));

                    hasmask = getmask(
                                  pVars->winmap[pChVars[LEFT]->wnd],
                                  pInputStream,
                                  pChLeftShare->group,
                                  pChLeftShare->max_sfb,
                                  pVars->mask);

                    if (hasmask == MASK_ERROR)
                    {
                        status = 1;
                    }
                    pVars->hasmask  = hasmask;

                }
            }
            else
            {
                pVars->hasmask  = 0;
            }

        }

    }

    ch = 0;
    while ((ch < num_channels) && (status == SUCCESS))
    {
        pChLeftShare = pChVars[ch]->pShareWfxpCoef;

        status = getics(
                     pInputStream,
                     common_window,
                     pVars,
                     pChVars[ch],
                     pChLeftShare->group,
                     &pChLeftShare->max_sfb,
                     pChLeftShare->cb_map,
                     &pChLeftShare->tns,
                     pVars->winmap,
                     &pVars->share.a.pulseInfo,
                     pVars->share.a.sect);

        ch++;

    }

    return status;

}

