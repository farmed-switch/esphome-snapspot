

#include "pv_audio_type_defs.h"
#include "s_bits.h"
#include "ibstream.h"
#include "e_rawbitstreamconst.h"
#include "getfill.h"

void getfill(BITS *pInputStream)
{

    Int cnt;
    Int esc_cnt;

    cnt = get9_n_lessbits(
              LEN_F_CNT,
              pInputStream);

    if (cnt == (1 << LEN_F_CNT) - 1)
    {
        esc_cnt = get9_n_lessbits(
                      LEN_F_ESC,
                      pInputStream);

        cnt +=  esc_cnt - 1;
    }

    pInputStream->usedBits += cnt * LEN_BYTE;

}

