

#include    "pv_audio_type_defs.h"
#include    "huffman.h"

Int huffcb(
    SectInfo    *pSect,
    BITS        *pInputStream,
    Int         sectbits[],
    Int         tot_sfb,
    Int         sfb_per_win,
    Int         max_sfb)
{

    Int   base;
    Int   sect_len_incr;
    Int   esc_val;
    Int     bits;
    Int     num_sect;
    Int     active_sfb;
    Int   group_base;

    bits       =  sectbits[0];
    esc_val    = (1 << bits) - 1;
    num_sect   =  0;
    base       =  0;
    group_base =  0;

    while ((base < tot_sfb) && (num_sect < tot_sfb))
    {

        pSect->sect_cb  = get9_n_lessbits(
                              LEN_CB,
                              pInputStream);

        sect_len_incr   = get9_n_lessbits(
                              bits,
                              pInputStream);

        while ((sect_len_incr == esc_val) && (base < tot_sfb))
        {
            base            +=  esc_val;

            sect_len_incr   = get9_n_lessbits(
                                  bits,
                                  pInputStream);
        }

        base      += sect_len_incr;
        pSect->sect_end  =  base;
        pSect++;
        num_sect++;

        active_sfb = base - group_base;

        if ((active_sfb == max_sfb) && (active_sfb < tot_sfb))
        {
            base      += (sfb_per_win - max_sfb);
            pSect->sect_cb   =   0;
            pSect->sect_end  =   base;
            num_sect++;
            pSect++;
            group_base = base;
        }
        else if (active_sfb > max_sfb)
        {

            break;
        }

    }

    if (base != tot_sfb || num_sect > tot_sfb)
    {
        num_sect = 0;
    }

    return num_sect;

}

