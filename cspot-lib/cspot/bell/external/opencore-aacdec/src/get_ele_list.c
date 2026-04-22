

#include "pv_audio_type_defs.h"
#include "s_elelist.h"
#include "s_bits.h"
#include "e_progconfigconst.h"
#include "ibstream.h"
#include "get_ele_list.h"

void get_ele_list(
    EleList     *pElementList,
    BITS        *pInputStream,
    const Bool   enableCPE)
{
    Int index;
    Int *pEleIsCPE;
    Int *pEleTag;

    pEleIsCPE = &pElementList->ele_is_cpe[0];
    pEleTag   = &pElementList->ele_tag[0];

    for (index = pElementList->num_ele; index > 0; index--)
    {
        if (enableCPE != FALSE)
        {
            *pEleIsCPE++ = get1bits( pInputStream);
        }
        else
        {
            *pEleIsCPE++ = FALSE;
        }

        *pEleTag++ = get9_n_lessbits(LEN_TAG, pInputStream);

    }

    return;

}

