

#include    "pv_audio_type_defs.h"
#include    "e_huffmanconst.h"
#include    "huffman.h"
#include    "aac_mem_funcs.h"
#include    "get_tns.h"

Int getics(
    BITS            *pInputStream,
    Int             common_window,
    tDec_Int_File   *pVars,
    tDec_Int_Chan   *pChVars,
    Int             group[],
    Int             *pMax_sfb,
    Int             *pCodebookMap,
    TNS_frame_info  *pTnsFrameInfo,
    FrameInfo       **pWinMap,
    PulseInfo       *pPulseInfo,
    SectInfo        sect[])
{

    Int     status = SUCCESS;

    Int     nsect = 0;
    Int     i;
    Int     cb;
    Int     sectWidth;
    Int     sectStart;
    Int     totSfb;
    Int     *pGroup;

    FrameInfo *pFrameInfo;

    Int     global_gain;
    Bool    present;

    pGroup = group;

    global_gain =
        get9_n_lessbits(
            LEN_SCL_PCM,
            pInputStream);

    if (common_window == FALSE)
    {
        status = get_ics_info(
                     pVars->mc_info.audioObjectType,
                     pInputStream,
                     common_window,
                     &pChVars->wnd,
                     &pChVars->wnd_shape_this_bk,
                     group,
                     pMax_sfb,
                     pWinMap,
                     &pChVars->pShareWfxpCoef->lt_status,
                     NULL);
    }

    pFrameInfo = pWinMap[pChVars->wnd];

    if (*pMax_sfb > 0)
    {

        i      = 0;
        totSfb = 0;

        do
        {
            totSfb++;

        }
        while (*pGroup++ < pFrameInfo->num_win);

        totSfb  *=  pFrameInfo->sfb_per_win[0];

        nsect =
            huffcb(
                sect,
                pInputStream,
                pFrameInfo->sectbits,
                totSfb,
                pFrameInfo->sfb_per_win[0],
                *pMax_sfb);

        if (nsect == 0)
        {
            status = 1;

        }

        sectStart = 0;
        for (i = 0; i < nsect; i++)
        {
            cb  = sect[i].sect_cb;
            sectWidth =  sect[i].sect_end - sectStart;
            sectStart += sectWidth;

            while (sectWidth > 0)
            {
                *pCodebookMap++ = cb;
                sectWidth--;
            }

        }

    }
    else
    {

        pv_memset(
            pCodebookMap,
            ZERO_HCB,
            MAXBANDS*sizeof(*pCodebookMap));

    }

    if (pFrameInfo->islong == FALSE)
    {
        calc_gsfb_table(
            pFrameInfo,
            group);
    }

    if (status == SUCCESS)
    {
        status =
            hufffac(
                pFrameInfo,
                pInputStream,
                group,
                nsect,
                sect,
                global_gain,
                pChVars->pShareWfxpCoef->factors,
                pVars->scratch.huffbook_used);

    }

    if (status == SUCCESS)
    {
        present =
            get1bits(pInputStream);

        pPulseInfo->pulse_data_present = present;

        if (present != FALSE)
        {
            if (pFrameInfo->islong == 1)
            {
                status = get_pulse_data(
                             pPulseInfo,
                             pInputStream);
            }
            else
            {

                status = 1;

            }
        }

    }

    if (status == SUCCESS)
    {
        present =
            get1bits(pInputStream);

        pTnsFrameInfo->tns_data_present = present;

        if (present != FALSE)
        {
            get_tns(
                pChVars->pShareWfxpCoef->max_sfb,
                pInputStream,
                pChVars->wnd,
                pFrameInfo,
                &pVars->mc_info,
                pTnsFrameInfo,
                pVars->scratch.tns_decode_coef);
        }
        else
        {
            for (i = pFrameInfo->num_win - 1; i >= 0 ; i--)
            {
                pTnsFrameInfo->n_filt[i] = 0;
            }

        }

    }

    if (status == SUCCESS)
    {
        present =
            get1bits(pInputStream);

        if (present != FALSE)
        {

            status = 1;
        }
    }

    if (status == SUCCESS)
    {
        status =
            huffspec_fxp(
                pFrameInfo,
                pInputStream,
                nsect,
                sect,
                pChVars->pShareWfxpCoef->factors,
                pChVars->fxpCoef,
                pVars->share.a.quantSpec,
                pVars->scratch.tmp_spec,
                pWinMap[ONLY_LONG_WINDOW],
                pPulseInfo,
                pChVars->pShareWfxpCoef->qFormat);
    }

    return status;

}
