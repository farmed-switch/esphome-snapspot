

#include "config.h"

#ifdef AAC_PLUS

#include    "extractframeinfo.h"
#include    "buf_getbits.h"
#include    "aac_mem_funcs.h"

const Int32 bs_pointer_bits_tbl[MAX_ENVELOPES + 1] = { 0, 1, 2, 2, 3, 3};

const Int32 T_16_ov_bs_num_env_tbl[MAX_ENVELOPES + 1] = { 2147483647, 16, 8,
        5,  4, 3
                                                        };

SBR_ERROR extractFrameInfo(BIT_BUFFER     * hBitBuf,
                           SBR_FRAME_DATA * h_frame_data)
{

    Int32 absBordLead = 0;
    Int32 nRelLead = 0;
    Int32 nRelTrail = 0;
    Int32 bs_num_env = 0;
    Int32 bs_num_rel = 0;
    Int32 bs_var_bord = 0;
    Int32 bs_var_bord_0 = 0;
    Int32 bs_var_bord_1 = 0;
    Int32 bs_pointer = 0;
    Int32 bs_pointer_bits;
    Int32 frameClass;
    Int32 temp;
    Int32 env;
    Int32 k;
    Int32 bs_num_rel_0 = 0;
    Int32 bs_num_rel_1 = 0;
    Int32 absBordTrail = 0;
    Int32 middleBorder = 0;
    Int32 bs_num_noise;
    Int32 lA = 0;

    Int32 tE[MAX_ENVELOPES + 1];
    Int32 tQ[2 + 1];
    Int32 f[MAX_ENVELOPES + 1];
    Int32 bs_rel_bord[3];
    Int32 bs_rel_bord_0[3];
    Int32 bs_rel_bord_1[3];
    Int32 relBordLead[3];
    Int32 relBordTrail[3];

    Int32 *v_frame_info = h_frame_data->frameInfo;

    SBR_ERROR err =  SBRDEC_OK;

    h_frame_data->frameClass = frameClass = buf_getbits(hBitBuf, SBR_CLA_BITS);

    switch (frameClass)
    {

        case FIXFIX:
            temp = buf_getbits(hBitBuf, SBR_ENV_BITS);

            bs_num_env = 1 << temp;

            f[0] = buf_getbits(hBitBuf, SBR_RES_BITS);

            for (env = 1; env < bs_num_env; env++)
            {
                f[env] = f[0];
            }

            nRelLead     = bs_num_env - 1;
            absBordTrail  = 16;

            break;

        case FIXVAR:
            bs_var_bord = buf_getbits(hBitBuf, SBR_ABS_BITS);
            bs_num_rel  = buf_getbits(hBitBuf, SBR_NUM_BITS);
            bs_num_env  = bs_num_rel + 1;

            for (k = 0; k < bs_num_env - 1; k++)
            {
                bs_rel_bord[k] = (buf_getbits(hBitBuf, SBR_REL_BITS) + 1) << 1;
            }

            bs_pointer_bits = bs_pointer_bits_tbl[bs_num_env];

            bs_pointer = buf_getbits(hBitBuf, bs_pointer_bits);

            for (env = 0; env < bs_num_env; env++)
            {
                f[bs_num_env - 1 - env] = buf_getbits(hBitBuf, SBR_RES_BITS);
            }

            absBordTrail  = 16 + bs_var_bord;
            nRelTrail     = bs_num_rel;

            break;

        case VARFIX:
            bs_var_bord = buf_getbits(hBitBuf, SBR_ABS_BITS);
            bs_num_rel  = buf_getbits(hBitBuf, SBR_NUM_BITS);
            bs_num_env  = bs_num_rel + 1;

            for (k = 0; k < bs_num_env - 1; k++)
            {
                bs_rel_bord[k] = (buf_getbits(hBitBuf, SBR_REL_BITS) + 1) << 1;
            }

            bs_pointer_bits = bs_pointer_bits_tbl[bs_num_env];

            bs_pointer = buf_getbits(hBitBuf, bs_pointer_bits);

            for (env = 0; env < bs_num_env; env++)
            {
                f[env] = buf_getbits(hBitBuf, SBR_RES_BITS);
            }

            absBordTrail = 16;
            absBordLead  = bs_var_bord;
            nRelLead     = bs_num_rel;

            break;

        case VARVAR:
            bs_var_bord_0 = buf_getbits(hBitBuf, SBR_ABS_BITS);
            bs_var_bord_1 = buf_getbits(hBitBuf, SBR_ABS_BITS);
            bs_num_rel_0  = buf_getbits(hBitBuf, SBR_NUM_BITS);
            bs_num_rel_1  = buf_getbits(hBitBuf, SBR_NUM_BITS);

            bs_num_env = bs_num_rel_0 + bs_num_rel_1 + 1;

            for (k = 0; k < bs_num_rel_0; k++)
            {
                bs_rel_bord_0[k] = (buf_getbits(hBitBuf, SBR_REL_BITS) + 1) << 1;
            }

            for (k = 0; k < bs_num_rel_1; k++)
            {
                bs_rel_bord_1[k] = (buf_getbits(hBitBuf, SBR_REL_BITS) + 1) << 1;
            }

            bs_pointer_bits = bs_pointer_bits_tbl[bs_num_env];

            bs_pointer = buf_getbits(hBitBuf, bs_pointer_bits);

            for (env = 0; env < bs_num_env; env++)
            {
                f[env] = buf_getbits(hBitBuf, SBR_RES_BITS);
            }

            absBordLead   = bs_var_bord_0;
            absBordTrail  = 16 + bs_var_bord_1;
            nRelLead      = bs_num_rel_0;
            nRelTrail     = bs_num_rel_1;

            break;

    };

    switch (frameClass)
    {
        case FIXFIX:
            for (k = 0; k < nRelLead; k++)
            {
                relBordLead[k] = T_16_ov_bs_num_env_tbl[bs_num_env];
            }
            break;
        case VARFIX:
            for (k = 0; k < nRelLead; k++)
            {
                relBordLead[k] = bs_rel_bord[k];
            }
            break;
        case VARVAR:
            for (k = 0; k < nRelLead; k++)
            {
                relBordLead[k] = bs_rel_bord_0[k];
            }
            for (k = 0; k < nRelTrail; k++)
            {
                relBordTrail[k] = bs_rel_bord_1[k];
            }
            break;
        case FIXVAR:
            for (k = 0; k < nRelTrail; k++)
            {
                relBordTrail[k] = bs_rel_bord[k];
            }
            break;
    }

    tE[0]          = absBordLead;
    tE[bs_num_env] = absBordTrail;

    for (env = 1; env <= nRelLead; env++)
    {
        tE[env] = absBordLead;
        for (k = 0; k <= env - 1; k++)
        {
            tE[env] += relBordLead[k];
        }
    }

    for (env = nRelLead + 1; env < bs_num_env; env++)
    {
        tE[env] = absBordTrail;
        for (k = 0; k <= bs_num_env - env - 1; k++)
        {
            tE[env] -= relBordTrail[k];
        }
    }

    switch (frameClass)
    {
        case  FIXFIX:
            middleBorder = bs_num_env >> 1;
            break;
        case VARFIX:
            switch (bs_pointer)
            {
                case 0:
                    middleBorder = 1;
                    break;
                case 1:
                    middleBorder = bs_num_env - 1;
                    break;
                default:
                    middleBorder = bs_pointer - 1;
                    break;
            };
            break;
        case FIXVAR:
        case VARVAR:
            switch (bs_pointer)
            {
                case 0:
                case 1:
                    middleBorder = bs_num_env - 1;
                    break;
                default:
                    middleBorder = bs_num_env + 1 - bs_pointer;
                    break;
            };
            break;
    };

    tQ[0] = tE[0];
    if (bs_num_env > 1)
    {
        tQ[1] = tE[middleBorder];
        tQ[2] = tE[bs_num_env];
        bs_num_noise = 2;
    }
    else
    {
        tQ[1] = tE[bs_num_env];
        bs_num_noise = 1;
    }

    if ((tE[bs_num_env] < tE[0]) || (tE[0] < 0))
    {
        err = SBRDEC_INVALID_BITSTREAM;
    }

    switch (frameClass)
    {
        case  FIXFIX:
            lA = -1;
            break;
        case VARFIX:
            switch (bs_pointer)
            {
                case 0:
                case 1:
                    lA = -1;
                    break;
                default:
                    lA = bs_pointer - 1;
                    break;
            };
            break;
        case FIXVAR:
        case VARVAR:
            switch (bs_pointer)
            {
                case 0:
                    lA = - 1;
                    break;
                default:
                    lA = bs_num_env + 1 - bs_pointer;
                    break;
            };
            break;
    };

    v_frame_info[0] = bs_num_env;
    pv_memcpy(v_frame_info + 1, tE, (bs_num_env + 1)*sizeof(Int32));

    pv_memcpy(v_frame_info + 1 + bs_num_env + 1, f, bs_num_env*sizeof(Int32));

    temp = (1 + bs_num_env) << 1;
    v_frame_info[temp] = lA;
    v_frame_info[temp + 1] = bs_num_noise;

    pv_memcpy(v_frame_info + temp + 2, tQ, (bs_num_noise + 1)*sizeof(Int32));

    return (err);

}

#endif

