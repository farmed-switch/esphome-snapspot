

#ifndef GET_ELE_LIST_H
#define GET_ELE_LIST_H

#include "pv_audio_type_defs.h"
#include "s_elelist.h"
#include "s_bits.h"

void get_ele_list(
    EleList     *pElementList,
    BITS        *pInputStream,
    const Bool   enableCPE);

#endif

