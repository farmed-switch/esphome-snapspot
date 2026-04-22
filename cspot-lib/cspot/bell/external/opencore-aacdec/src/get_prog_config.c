

#include "pv_audio_type_defs.h"
#include "s_bits.h"
#include "s_elelist.h"
#include "s_tdec_int_file.h"
#include "s_tdec_int_chan.h"
#include "e_progconfigconst.h"
#include "ibstream.h"
#include "get_ele_list.h"
#include "aac_mem_funcs.h"
#include "set_mc_info.h"
#include "get_prog_config.h"

Int get_prog_config(
    tDec_Int_File *pVars,
    ProgConfig    *pScratchPCE)
{
    Int    i;
    UInt    tag;
    Int    numChars;
    UInt    temp;
    Bool   flag;
    Int    status          = SUCCESS;
    BITS  *pInputStream   = &(pVars->inputStream);

    tag =
        get9_n_lessbits(
            LEN_TAG,
            pInputStream);

    pScratchPCE->profile =
        get9_n_lessbits(
            LEN_PROFILE,
            pInputStream);

    pScratchPCE->sampling_rate_idx =
        get9_n_lessbits(
            LEN_SAMP_IDX,
            pInputStream);

    if (!pVars->adif_test && pScratchPCE->sampling_rate_idx != pVars->prog_config.sampling_rate_idx)
    {

        pInputStream->usedBits -= (LEN_TAG + LEN_PROFILE + LEN_SAMP_IDX);

        return (1);
    }

    temp =
        get9_n_lessbits(
            LEN_NUM_ELE,
            pInputStream);

    pScratchPCE->front.num_ele = temp;

    temp =
        get9_n_lessbits(
            LEN_NUM_ELE,
            pInputStream);

    pScratchPCE->side.num_ele = temp;

    temp =
        get9_n_lessbits(
            LEN_NUM_ELE,
            pInputStream);

    pScratchPCE->back.num_ele = temp;

    temp =
        get9_n_lessbits(
            LEN_NUM_LFE,
            pInputStream);

    pScratchPCE->lfe.num_ele = temp;

    temp =
        get9_n_lessbits(
            LEN_NUM_DAT,
            pInputStream);
    pScratchPCE->data.num_ele = temp;

    temp =
        get9_n_lessbits(
            LEN_NUM_CCE,
            pInputStream);

    pScratchPCE->coupling.num_ele = temp;

    flag =
        get1bits(
            pInputStream);

    pScratchPCE->mono_mix.present = flag;

    if (flag != FALSE)
    {
        temp =
            get9_n_lessbits(
                LEN_TAG,
                pInputStream);

        pScratchPCE->mono_mix.ele_tag = temp;

    }

    flag =
        get1bits(
            pInputStream);

    pScratchPCE->stereo_mix.present = flag;

    if (flag != FALSE)
    {
        temp =
            get9_n_lessbits(
                LEN_TAG,
                pInputStream);

        pScratchPCE->stereo_mix.ele_tag = temp;

    }

    flag =
        get1bits(
            pInputStream);

    pScratchPCE->matrix_mix.present = flag;

    if (flag != FALSE)
    {
        temp =
            get9_n_lessbits(
                LEN_MMIX_IDX,
                pInputStream);

        pScratchPCE->matrix_mix.ele_tag = temp;

        temp =
            get1bits(
                pInputStream);

        pScratchPCE->matrix_mix.pseudo_enab = temp;

    }

    get_ele_list(
        &pScratchPCE->front,
        pInputStream,
        TRUE);

    get_ele_list(
        &pScratchPCE->side,
        pInputStream,
        TRUE);

    get_ele_list(
        &pScratchPCE->back,
        pInputStream,
        TRUE);

    get_ele_list(
        &pScratchPCE->lfe,
        pInputStream,
        FALSE);

    get_ele_list(
        &pScratchPCE->data,
        pInputStream,
        FALSE);

    get_ele_list(
        &pScratchPCE->coupling,
        pInputStream,
        TRUE);

    byte_align(pInputStream);

    numChars =
        get9_n_lessbits(
            LEN_COMMENT_BYTES, pInputStream);

    for (i = numChars; i > 0; i--)
    {
        pScratchPCE->comments[i] = (Char) get9_n_lessbits(LEN_BYTE,
                                   pInputStream);

    }

    if (pVars->current_program < 0)
    {

        pVars->current_program = tag;

    }

    if (tag == (UInt)pVars->current_program)
    {

        pv_memcpy(
            &pVars->prog_config,
            pScratchPCE,
            sizeof(ProgConfig));

        status =
            set_mc_info(
                &pVars->mc_info,
                (tMP4AudioObjectType)(pVars->prog_config.profile + 1),
                pVars->prog_config.sampling_rate_idx,
                pVars->prog_config.front.ele_tag[0],
                pVars->prog_config.front.ele_is_cpe[0],
                pVars->winmap,
                pVars->SFBWidth128);

    }

    return (status);
}

