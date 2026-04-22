

#include    "config.h"

#ifdef AAC_PLUS

#include    "sbr_aliasing_reduction.h"
#include    "pv_sqrt.h"

#include    "aac_mem_funcs.h"

#include    "pv_div.h"
#include    "fxp_mul32.h"

#define Q30fmt(x)   (Int32)(x*((Int32)1<<30) + (x>=0?0.5F:-0.5F))

#include "pv_normalize.h"
#include  "sbr_constants.h"

void sbr_aliasing_reduction(Int32 *degreeAlias,
                            Int32  * nrg_gain_man,
                            Int32  * nrg_gain_exp,
                            Int32  * nrg_est_man,
                            Int32  * nrg_est_exp,
                            Int32  * dontUseTheseGainValues,
                            Int32    noSubbands,
                            Int32    lowSubband,
                            Int32  sqrt_cache[][4],
                            Int32 * groupVector)
{

    Int32 temp1;
    Int32 est_total;
    Int32 ref_total_man;
    Int32 ref_total_exp;
    Int32 tmp_q1;
    Int32 tmp_q2;
    Int32 tmp_q3;
    Int32 tmp_q4;
    Int32 bst_man;
    Int32 bst_exp;
    struct intg_div   quotient;
    struct intg_sqrt  root_sq;
    Int32 group;
    Int32 grouping = 0;
    Int32 index = 0;
    Int32 noGroups;
    Int32 k;

    for (k = 0; k < noSubbands - 1; k++)
    {
        if (degreeAlias[k + lowSubband + 1] && dontUseTheseGainValues[k] == 0)
        {
            if (grouping == 0)
            {
                groupVector[index] = k + lowSubband;
                grouping = 1;
                index++;
            }
        }
        else
        {
            if (grouping)
            {
                groupVector[index] = k + lowSubband;

                if (! dontUseTheseGainValues[k])
                {
                    (groupVector[index])++;
                }
                grouping = 0;
                index++;
            }
        }
    }

    if (grouping)
    {
        groupVector[index] = noSubbands + lowSubband;
        index++;
    }
    noGroups = (index >> 1);

    for (group = 0; group < noGroups; group ++)
    {

        int startGroup = groupVector[(group<<1)] - lowSubband;
        int stopGroup  = groupVector[(group<<1)+1] - lowSubband;

        est_total = 0;
        ref_total_man = 0;

        tmp_q1 = -100;
        tmp_q2 = -100;

        for (k = startGroup; k < stopGroup; k++)
        {
            if (tmp_q1 < nrg_est_exp[k])
            {
                tmp_q1 = nrg_est_exp[k];
            }
            if (tmp_q2 < (nrg_est_exp[k] + (nrg_gain_exp[k] << 1)))
            {
                tmp_q2 = (nrg_est_exp[k] + (nrg_gain_exp[k] << 1));
            }
        }

        k -= startGroup;

        tmp_q2 += 59 - pv_normalize(k);

        for (k = startGroup; k < stopGroup; k++)
        {

            est_total += nrg_est_man[k] >> (tmp_q1 - nrg_est_exp[k]);

            if (tmp_q2 - (nrg_est_exp[k] + (nrg_gain_exp[k] << 1)) < 60)
            {
                nrg_gain_man[k] = fxp_mul32_Q28(nrg_gain_man[k], nrg_gain_man[k]);
                nrg_gain_exp[k] = (nrg_gain_exp[k] << 1) + 28;
                tmp_q3          = fxp_mul32_Q28(nrg_gain_man[k], nrg_est_man[k]);
                ref_total_man    += tmp_q3 >> (tmp_q2 - (nrg_est_exp[k] + nrg_gain_exp[k]));
            }
        }

        ref_total_exp = tmp_q2 + 28;

        pv_div(ref_total_man, est_total, &quotient);

        tmp_q2 += - tmp_q1 - quotient.shift_factor - 2;

        for (k = startGroup; k < stopGroup; k++)
        {
            Int32 alpha;
            temp1 = k + lowSubband;
            if (k < noSubbands - 1)
            {
                alpha = degreeAlias[temp1 + 1] > degreeAlias[temp1 ] ?
                        degreeAlias[temp1 + 1] : degreeAlias[temp1 ];
            }
            else
            {
                alpha = degreeAlias[temp1];
            }

            tmp_q1 = tmp_q2 > nrg_gain_exp[k] ? tmp_q2 : nrg_gain_exp[k];
            tmp_q1++;

            tmp_q3 = fxp_mul32_Q30(alpha, quotient.quotient);
            tmp_q4 = fxp_mul32_Q30(Q30fmt(1.0f) - alpha, nrg_gain_man[k]);

            nrg_gain_man[k] = (tmp_q3 >> (tmp_q1 - tmp_q2)) +
                              (tmp_q4 >> (tmp_q1 - nrg_gain_exp[k]));

            nrg_gain_exp[k] = tmp_q1;
        }

        bst_exp = -100;

        for (k = startGroup; k < stopGroup; k++)
        {
            if (bst_exp < nrg_gain_exp[k] + nrg_est_exp[k])
            {
                bst_exp = nrg_gain_exp[k] + nrg_est_exp[k];
            }
        }

        k -= startGroup;

        while (k != 0)
        {
            k >>= 1;
            bst_exp++;
        }

        bst_man = 0;

        for (k = startGroup; k < stopGroup; k++)
        {
            tmp_q2 =  fxp_mul32_Q28(nrg_gain_man[k], nrg_est_man[k]);
            bst_man +=  tmp_q2 >> (bst_exp - nrg_gain_exp[k] - nrg_est_exp[k]);
        }

        bst_exp += 28;

        if (bst_man)
        {

            pv_div(ref_total_man, bst_man, &quotient);
            bst_exp = ref_total_exp - bst_exp - quotient.shift_factor - 30;
            bst_man = quotient.quotient;

            for (k = startGroup; k < stopGroup; k++)
            {
                tmp_q1 = fxp_mul32_Q30(bst_man, nrg_gain_man[k]);
                pv_sqrt(tmp_q1, (bst_exp + nrg_gain_exp[k] + 60), &root_sq, sqrt_cache[0]);
                nrg_gain_man[k] = root_sq.root;
                nrg_gain_exp[k] = root_sq.shift_factor;
            }
        }
        else
        {
            pv_memset((void *)&nrg_gain_man[startGroup],
                      0,
                      (stopGroup - startGroup)*sizeof(nrg_gain_man[0]));

            pv_memset((void *)&nrg_gain_exp[startGroup],
                      0,
                      (stopGroup - startGroup)*sizeof(nrg_gain_exp[0]));

        }

    }
}

#endif

