

#include "config.h"

#ifdef AAC_PLUS

#include    "calc_sbr_envelope.h"
#include    "sbr_envelope_calc_tbl.h"
#include    "sbr_create_limiter_bands.h"
#include    "aac_mem_funcs.h"

#include    "fxp_mul32.h"
#include    "pv_normalize.h"

#include    "sbr_aliasing_reduction.h"

#include    "pv_sqrt.h"

#include    "pv_div.h"
#include    "fxp_mul32.h"
#include    "pv_normalize.h"

#define Q30fmt(x)   (Int32)(x*((Int32)1<<30) + (x>=0?0.5F:-0.5F))
#define Q28fmt(x)   (Int32)(x*((Int32)1<<28) + (x>=0?0.5F:-0.5F))
#define Q15fmt(x)   (Int32)(x*((Int32)1<<15) + (x>=0?0.5F:-0.5F))

#ifdef __cplusplus
extern "C"
{
#endif

    void envelope_application_LC(Int32  *aBufR,
    Int32  *nrg_gain_man,
    Int32  *nrg_gain_exp,
    Int32  *noise_level_man,
    Int32  *noise_level_exp,
    Int32  *nrg_tone_man,
    Int32  *nrg_tone_exp,
    Int32  band_nrg_tone_detector,
    const Int32 *frame_info,
    Int32  *harm_index,
    Int32  *phase_index,
    Int32  i,
    Int32  lowSubband,
    Int32  noSubbands,
    Int32  noNoiseFlag);

    void energy_estimation_LC(Int32 *aBufR,
                              Int32 *nrg_est_man,
                              Int32 *nrg_est_exp,
                              const Int32 *frame_info,
                              Int32 i,
                              Int32 k,
                              Int32 c,
                              Int32 ui2);

#ifdef HQ_SBR

    void envelope_application(Int32  *aBufR,
                              Int32  *aBufI,
                              Int32  *nrg_gain_man,
                              Int32  *nrg_gain_exp,
                              Int32  *noise_level_man,
                              Int32  *noise_level_exp,
                              Int32  *nrg_tone_man,
                              Int32  *nrg_tone_exp,
                              Int32 *fBuf_man[64],
                              Int32 *fBuf_exp[64],
                              Int32 *fBufN_man[64],
                              Int32 *fBufN_exp[64],
                              const Int32 *frame_info,
                              Int32  *harm_index,
                              Int32  *phase_index,
                              Int32  i,
                              Int32  lowSubband,
                              Int32  noSubbands,
                              Int32  noNoiseFlag,
                              Int32  band_nrg_tone_detector,
                              Int32  maxSmoothLength,
                              Int32  smooth_length);

    void energy_estimation(Int32 *aBufR,
                           Int32 *aBufI,
                           Int32 *nrg_est_man,
                           Int32 *nrg_est_exp,
                           const Int32 *frame_info,
                           Int32 i,
                           Int32 k,
                           Int32 c,
                           Int32 ui2);

#endif

#ifdef __cplusplus
}
#endif

void calc_sbr_envelope(SBR_FRAME_DATA *frameData,
                       Int32 *aBufR,
                       Int32 *aBufI,
                       Int freqBandTable1[2][MAX_FREQ_COEFFS + 1],
                       const Int32 *nSfb,
                       Int32 freqBandTable2[MAX_NOISE_COEFFS + 1],
                       Int32 nNBands,
                       Int32 reset,
                       Int32 *degreeAlias,
                       Int32 *harm_index,
                       Int32 *phase_index,
                       Int32 hFp[64],
                       Int32 *sUp,
                       Int32 limSbc[][13],
                       Int32 *gateMode,
#ifdef HQ_SBR
                       Int32 *fBuf_man[64],
                       Int32 *fBuf_exp[64],
                       Int32 *fBufN_man[64],
                       Int32 *fBufN_exp[64],
#endif
                       Int32 scratch_mem[][64],
                       struct PATCH Patch,
                       Int32  sqrt_cache[][4],
                       Int32  LC_flag)
{

    Int32 c;
    Int32 li;
    Int32 ui;
    Int32 i;
    Int32 j;
    Int32 k = 0;
    Int32 l;
    Int m = 0;
    Int kk = 0;
    Int o;
    Int next = -1;
    Int32 ui2;
    Int flag;
    Int noNoiseFlag;
    Int *ptr;

    UInt32 nrg = 0;
    Int32 nrg_exp = 0;
    struct intg_div   quotient;
    struct intg_sqrt  root_sq;

    Int32 aux1;

    Int32 *nL_man       = frameData->sbrNoiseFloorLevel_man;
    Int32 *nL_exp       = frameData->sbrNoiseFloorLevel_exp;

    Int32 *sfb_nrg_man  = frameData->iEnvelope_man;
    Int32 *sfb_nrg_exp  = frameData->iEnvelope_exp;

    Int32 tmp_q1;
    Int32 tmp_q2;

    Int32 g_max_man;
    Int32 g_max_exp;

    Int32 p_ref_man;
    Int32 p_ref_exp;

    Int32 p_est_man;
    Int32 p_est_exp;

    Int32 p_adj_man;
    Int32 p_adj_exp;
    Int32 avg_gain;

    Int32 boost_gain_q;

    Int32 band_nrg_tone_detector;

    Int32 *nrg_est_man     = scratch_mem[0];
    Int32 *nrg_est_exp     = scratch_mem[1];
    Int32 *nrg_ref_man     = scratch_mem[2];
    Int32 *nrg_ref_exp     = scratch_mem[3];
    Int32 *nrg_gain_man    = scratch_mem[4];
    Int32 *nrg_gain_exp    = scratch_mem[5];
    Int32 *noise_level_man = scratch_mem[6];
    Int32 *noise_level_exp = scratch_mem[7];
    Int32 *nrg_tone_man    = scratch_mem[8];
    Int32 *nrg_tone_exp    = scratch_mem[9];
    Int32 *hF              = scratch_mem[10];

    const Int32 *frame_info = frameData->frameInfo;
    Int32 int_mode          = frameData->sbr_header.interpolFreq;

    Int32 dontUseTheseGainValues[64];

#ifdef HQ_SBR

    Int32 n;
    Int32 smooth_length;
    Int32 smoothingLength   = frameData->sbr_header.smoothingLength;
    Int32 maxSmoothLength   = smoothLengths[0];

#endif

    Int32 limiterBand       = frameData->sbr_header.limiterBands;
    Int32 limiterGains      = frameData->sbr_header.limiterGains;
    Int32 *addHarmonics     = frameData->addHarmonics;

    Int32 lowSubband        = freqBandTable1[LOW_RES][0];
    Int32 noSubbands        = freqBandTable1[LOW_RES][nSfb[LOW_RES]] - lowSubband;
    Int32 nEnv              = frame_info[0];
    Int32 sEnv              = frame_info[(nEnv + 1)<<1];

    noSubbands = (noSubbands >> 31) ^ noSubbands;
    if (noSubbands > 64)
    {
        noSubbands = 64;
    }

    if (reset)
    {
        *sUp = 1;
        *phase_index = 0;
        sbr_create_limiter_bands(limSbc,
                                 gateMode,
                                 freqBandTable1[LOW_RES],
                                 Patch,
                                 nSfb[LOW_RES]);
    }

    pv_memset((void*)hF, 0, (sizeof(*hF) << 6));

    ptr  = freqBandTable1[HI];
    l = *(ptr++);

    for (i = nSfb[HI]; i != 0; i--)
    {
        k     = *(ptr++);
        j     = ((k + l) >> 1) - lowSubband;
        l   = k;
        hF[j] = *(addHarmonics++);
    }

    for (i = 0; i < nEnv; i++)
    {

        if (frame_info[1+i] == frame_info[(nEnv<<1)+4+kk])
        {
            kk++, next++;
        }

        noNoiseFlag = (i == sEnv || i == frameData->prevEnvIsShort) ? 1 : 0;

#ifdef HQ_SBR
        smooth_length = (noNoiseFlag ? 0 : smoothLengths[smoothingLength]);
#endif

        c = 0;
        o = 0;

        band_nrg_tone_detector = 0;

        Int kkkk = freqBandTable1[ frame_info[nEnv+2+i] ][0];

        for (j = 0; j <  nSfb[frame_info[nEnv+2+i]]; j++)
        {
            li = freqBandTable1[ frame_info[nEnv+2+i] ][j    ];
            ui = freqBandTable1[ frame_info[nEnv+2+i] ][j + 1];
            flag = 0;

            for (k = li; k < ui; k++)
            {
                ui2   = (frame_info[1+i] << 1);

                if (LC_flag == ON)
                {
                    energy_estimation_LC((Int32 *)aBufR,
                                         nrg_est_man,
                                         nrg_est_exp,
                                         frame_info,
                                         i,
                                         k - kkkk,
                                         c,
                                         ui2);
                }
#ifdef HQ_SBR
                else
                {

                    energy_estimation((Int32 *)aBufR,
                                      (Int32 *)aBufI,
                                      nrg_est_man,
                                      nrg_est_exp,
                                      frame_info,
                                      i,
                                      k - kkkk,
                                      c,
                                      ui2);
                }
#endif

                flag = (hF[c] && (i >= sEnv || hFp[c+lowSubband])) ? 1 : flag;
                c++;
            }

            ui2 = freqBandTable2[o+1];

            if (!int_mode)
            {

                tmp_q1 = -100;

                for (k = c - (ui - li); k < c; k++)
                {
                    if (tmp_q1 < nrg_est_exp[k])
                    {
                        tmp_q1 = nrg_est_exp[k];
                    }
                }

                nrg = 0;
                for (k = c - (ui - li); k < c; k++)
                {
                    nrg += nrg_est_man[k] >> (tmp_q1 - nrg_est_exp[k]);
                }
                nrg /= (ui - li);
                nrg_exp = tmp_q1;

            }

            c -= (ui - li);

            for (k = 0; k < ui - li; k++)
            {
                o = (k + li >= ui2) ? o + 1 : o;
                ui2 = freqBandTable2[o+1];

                if (!int_mode)
                {
                    nrg_est_man[c] = nrg;
                    nrg_est_exp[c] = nrg_exp;
                }

                if (LC_flag == ON)
                {
                    nrg_est_exp[c] += 1;

                    if (flag)
                    {
                        dontUseTheseGainValues[k + li - lowSubband] = 1;
                    }
                    else
                    {
                        dontUseTheseGainValues[k + li - lowSubband] = 0;
                    }
                }

                nrg_ref_man[c] = sfb_nrg_man[m];
                nrg_ref_exp[c] = sfb_nrg_exp[m];

                aux1 = next * nNBands + o;

                tmp_q1 = nL_exp[aux1];

                if (tmp_q1 >= 0)
                {
                    pv_div(nL_man[aux1], nL_man[aux1] + (0x3FFFFFFF >> tmp_q1), &quotient);
                }
                else
                {
                    tmp_q1 = nL_man[aux1] >> (-tmp_q1);
                    pv_div(tmp_q1, tmp_q1 + 0x3FFFFFFF, &quotient);
                }

                tmp_q1 = fxp_mul32_Q30(quotient.quotient >> quotient.shift_factor,  nrg_ref_man[c]);

                if (flag)
                {

                    pv_div(tmp_q1, nrg_est_man[c] + 1, &quotient);

                    tmp_q2 = nrg_ref_exp[c] - nrg_est_exp[c] - quotient.shift_factor - 30;

                    pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[1]);
                    nrg_gain_man[c] = root_sq.root;
                    nrg_gain_exp[c] = root_sq.shift_factor;

                    if (hF[c] && (i >= sEnv || hFp[c+lowSubband]))
                    {

                        tmp_q2 = nL_exp[aux1];

                        if (tmp_q2 >= 0)
                        {
                            pv_div(nrg_ref_man[c], nL_man[aux1] + (0x3FFFFFFF >> tmp_q2), &quotient);
                        }
                        else
                        {
                            tmp_q2 = nL_man[aux1] >> (-tmp_q2);
                            pv_div(nrg_ref_man[c], tmp_q2 + 0x3FFFFFFF, &quotient);
                            tmp_q2 = 0;
                        }

                        tmp_q2 = nrg_ref_exp[c] - tmp_q2 - quotient.shift_factor;

                        pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[2]);
                        nrg_tone_man[c]    = root_sq.root;
                        nrg_tone_exp[c]    = root_sq.shift_factor;

                    }
                    else
                    {
                        nrg_tone_man[c]    = 0;
                        nrg_tone_exp[c]    = 0;
                    }

                }
                else
                {
                    if (noNoiseFlag)
                    {

                        pv_div(nrg_ref_man[c], nrg_est_man[c] + 1, &quotient);

                        tmp_q2 = nrg_ref_exp[c] - nrg_est_exp[c] - quotient.shift_factor - 30;

                        pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[3]);
                        nrg_gain_man[c] = root_sq.root;
                        nrg_gain_exp[c] = root_sq.shift_factor;

                    }
                    else
                    {

                        tmp_q2 = nL_exp[aux1];

                        if (nrg_est_man[c] == 0)
                        {
                            tmp_q2 = 0;
                        }

                        if (tmp_q2 >= 0)
                        {

                            tmp_q2 = fxp_mul32_Q30(nrg_est_man[c] + 1, nL_man[aux1] + (0x3FFFFFFF >> tmp_q2));
                            pv_div(nrg_ref_man[c], tmp_q2, &quotient);

                            tmp_q2 = nrg_ref_exp[c] - quotient.shift_factor - 30 - nL_exp[aux1];
                            if (nrg_est_man[c])
                            {
                                tmp_q2 -=  nrg_est_exp[c];
                            }

                            tmp_q2 = nrg_ref_exp[c] - nrg_est_exp[c] - quotient.shift_factor - 30 - nL_exp[aux1];
                        }
                        else
                        {
                            if (tmp_q2 > - 10)
                            {
                                tmp_q2 = nL_man[aux1] >> (-tmp_q2);

                                tmp_q2 = fxp_mul32_Q30(nrg_est_man[c] + 1, tmp_q2 + 0x3FFFFFFF);
                            }
                            else
                            {
                                tmp_q2 = nrg_est_man[c] + 1;
                            }

                            pv_div(nrg_ref_man[c], tmp_q2, &quotient);

                            tmp_q2 = nrg_ref_exp[c] - quotient.shift_factor - 30;
                            if (nrg_est_man[c])
                            {
                                tmp_q2 -=  nrg_est_exp[c];
                            }

                        }

                        pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[4]);
                        nrg_gain_man[c] = root_sq.root;
                        nrg_gain_exp[c] = root_sq.shift_factor;

                    }

                    nrg_tone_man[c]    = 0;
                    nrg_tone_exp[c]    = -100;

                }

                band_nrg_tone_detector |= nrg_tone_man[c];

                pv_sqrt(tmp_q1, nrg_ref_exp[c], &root_sq, sqrt_cache[5]);
                noise_level_man[c] = root_sq.root;
                noise_level_exp[c] = root_sq.shift_factor;

                c++;

            }
            m++;

        }

        for (c = 0; c < gateMode[limiterBand]; c++)
        {

            p_ref_man = 0;
            p_est_man = 0;

            p_ref_exp = -100;
            p_est_exp = -100;

            for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
            {
                if (p_ref_exp < nrg_ref_exp[k])
                {
                    p_ref_exp = nrg_ref_exp[k];
                }
                if (p_est_exp < nrg_est_exp[k])
                {
                    p_est_exp = nrg_est_exp[k];
                }
            }

            k -= limSbc[limiterBand][c];

            while (k != 0)
            {
                k >>= 1;
                p_ref_exp++;
            }

            for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
            {
                p_ref_man += (nrg_ref_man[k] >> (p_ref_exp - nrg_ref_exp[k]));
                p_est_man += (nrg_est_man[k] >> (p_est_exp - nrg_est_exp[k]));

            }

            if (p_est_man)
            {

                pv_div(p_ref_man, p_est_man, &quotient);

                tmp_q2 = p_ref_exp - 30 - p_est_exp - quotient.shift_factor;

                pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[6]);
                avg_gain  = root_sq.root;
                g_max_exp = root_sq.shift_factor;

                g_max_man = fxp_mul32_Q30(avg_gain, limGains[limiterGains]);

                if (limiterGains == 3)
                {
                    g_max_exp = limGains[4];
                }

                tmp_q1 = g_max_exp >= 16 ? g_max_exp : 16;

                tmp_q2 = g_max_man >> (tmp_q1 - g_max_exp);
                tmp_q1 = Q28fmt(1.52587890625F) >> (tmp_q1 - 16);

                if (tmp_q2 > tmp_q1)
                {

                    g_max_man = Q28fmt(1.52587890625F);
                    g_max_exp = 16;
                }
            }
            else
            {

                g_max_man = Q28fmt(1.52587890625F);
                g_max_exp = 16;
            }

            for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
            {

                tmp_q1 = g_max_exp >= nrg_gain_exp[k] ? g_max_exp : nrg_gain_exp[k];

                tmp_q2 = g_max_man >> (tmp_q1 - g_max_exp);
                tmp_q1 = nrg_gain_man[k] >> (tmp_q1 - nrg_gain_exp[k]);

                if (tmp_q2 <= tmp_q1)
                {
                    tmp_q1 = fxp_mul32_Q28(noise_level_man[k], g_max_man);
                    pv_div(tmp_q1, nrg_gain_man[k], &quotient);
                    noise_level_man[k] = quotient.quotient >> 2;
                    noise_level_exp[k] = noise_level_exp[k] + g_max_exp - quotient.shift_factor - nrg_gain_exp[k];

                    nrg_gain_man[k] =  g_max_man;
                    nrg_gain_exp[k] =  g_max_exp;
                }
            }

            p_adj_exp = -100;

            for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
            {
                tmp_q1 = nrg_est_exp[k] + (nrg_gain_exp[k] << 1) + 28;

                if (p_adj_exp < tmp_q1)
                {
                    p_adj_exp = tmp_q1;
                }
                if (nrg_tone_man[k])
                {
                    tmp_q1 = (nrg_tone_exp[k] << 1);
                    if (p_adj_exp < tmp_q1)
                    {
                        p_adj_exp = tmp_q1;
                    }
                }
                else if (!noNoiseFlag)
                {
                    tmp_q1 = (noise_level_exp[k] << 1);

                    if (p_adj_exp < tmp_q1)
                    {
                        p_adj_exp = tmp_q1;
                    }
                }
            }

            p_adj_exp += 1;

            p_adj_man = 0;

            for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
            {

                if (p_adj_exp - (nrg_est_exp[k] + (nrg_gain_exp[k] << 1)) < 59)
                {
                    tmp_q1 = fxp_mul32_Q28(nrg_gain_man[k], nrg_gain_man[k]);
                    tmp_q1 = fxp_mul32_Q28(tmp_q1, nrg_est_man[k]);
                    p_adj_man += (tmp_q1 >> (p_adj_exp - (nrg_est_exp[k] + (nrg_gain_exp[k] << 1) + 28)));
                }

                if (nrg_tone_man[k])
                {

                    if (p_adj_exp - (nrg_tone_exp[k] << 1) < 31)
                    {
                        tmp_q1 = fxp_mul32_Q28(nrg_tone_man[k], nrg_tone_man[k]);
                        p_adj_man += (tmp_q1 >> (p_adj_exp - (nrg_tone_exp[k] << 1)));
                    }
                }
                else if (!noNoiseFlag)
                {

                    if (p_adj_exp - (noise_level_exp[k] << 1) < 31)
                    {
                        tmp_q1 = fxp_mul32_Q28(noise_level_man[k], noise_level_man[k]);
                        p_adj_man += (tmp_q1 >> (p_adj_exp - (noise_level_exp[k] << 1)));
                    }

                }
            }

            if (p_adj_man)
            {
                pv_div(p_ref_man, p_adj_man, &quotient);
                tmp_q2 = p_ref_exp - p_adj_exp - 58 - quotient.shift_factor;

                pv_sqrt(quotient.quotient, tmp_q2, &root_sq, sqrt_cache[7]);

                if (root_sq.shift_factor > -28)
                {
                    boost_gain_q = root_sq.root << (root_sq.shift_factor + 28);
                }
                else
                {
                    boost_gain_q = root_sq.root >> (-28 - root_sq.shift_factor);
                }

                tmp_q1 = root_sq.shift_factor >= -28 ? root_sq.shift_factor : -28;

                tmp_q2 = root_sq.root >> (tmp_q1 - root_sq.shift_factor);
                tmp_q1 = Q28fmt(1.584893192f) >> (tmp_q1 + 28);

                if (tmp_q2 > tmp_q1)
                {
                    boost_gain_q = Q28fmt(1.584893192f);
                }
            }
            else
            {
                boost_gain_q = Q28fmt(1.584893192f);
            }

            if (band_nrg_tone_detector)
            {
                for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
                {
                    nrg_gain_man[k]    = fxp_mul32_Q28(nrg_gain_man[k], boost_gain_q);
                    noise_level_man[k] = fxp_mul32_Q28(noise_level_man[k], boost_gain_q);
                    nrg_tone_man[k]    = fxp_mul32_Q28(nrg_tone_man[k], boost_gain_q);
                }
            }
            else
            {

                for (k = limSbc[limiterBand][c]; k < limSbc[limiterBand][c + 1]; k++)
                {
                    nrg_gain_man[k]    = fxp_mul32_Q28(nrg_gain_man[k], boost_gain_q);
                    noise_level_man[k] = fxp_mul32_Q28(noise_level_man[k], boost_gain_q);
                }

            }

        }

        if (LC_flag == ON)
        {

            sbr_aliasing_reduction(degreeAlias,
                                   nrg_gain_man,
                                   nrg_gain_exp,
                                   nrg_est_man,
                                   nrg_est_exp,
                                   dontUseTheseGainValues,
                                   noSubbands,
                                   lowSubband,
                                   sqrt_cache,
                                   scratch_mem[3]);

            if (*sUp)
            {
                *sUp = 0;
            }

            envelope_application_LC((Int32 *)aBufR,
                                    nrg_gain_man,
                                    nrg_gain_exp,
                                    noise_level_man,
                                    noise_level_exp,
                                    nrg_tone_man,
                                    nrg_tone_exp,
                                    band_nrg_tone_detector,
                                    frame_info,
                                    harm_index,
                                    phase_index,
                                    i,
                                    lowSubband,
                                    noSubbands,
                                    noNoiseFlag);
        }
#ifdef HQ_SBR
        else
        {

            if (*sUp)
            {
                for (n = 0; n < maxSmoothLength; n++)
                {
                    pv_memcpy(fBuf_man[n],     nrg_gain_man, noSubbands*sizeof(*fBuf_man[n]));
                    pv_memcpy(fBufN_man[n], noise_level_man, noSubbands*sizeof(*fBufN_man[n]));
                    pv_memcpy(fBuf_exp[n],     nrg_gain_exp, noSubbands*sizeof(*fBuf_exp[n]));
                    pv_memcpy(fBufN_exp[n], noise_level_exp, noSubbands*sizeof(*fBufN_exp[n]));
                }
                *sUp = 0;
            }

            envelope_application((Int32 *)aBufR,
                                 (Int32 *)aBufI,
                                 nrg_gain_man,
                                 nrg_gain_exp,
                                 noise_level_man,
                                 noise_level_exp,
                                 nrg_tone_man,
                                 nrg_tone_exp,
                                 fBuf_man,
                                 fBuf_exp,
                                 fBufN_man,
                                 fBufN_exp,
                                 frame_info,
                                 harm_index,
                                 phase_index,
                                 i,
                                 lowSubband,
                                 noSubbands,
                                 noNoiseFlag,
                                 band_nrg_tone_detector,
                                 maxSmoothLength,
                                 smooth_length);

        }
#endif

    }

    pv_memcpy(&hFp[0] + lowSubband,
              hF,
              (64 - lowSubband)*sizeof(*hF));

    if (sEnv == nEnv)
    {
        frameData->prevEnvIsShort = 0;
    }
    else
    {
        frameData->prevEnvIsShort = -1;
    }

}

void envelope_application_LC(Int32  *aBufR,
                             Int32  *nrg_gain_man,
                             Int32  *nrg_gain_exp,
                             Int32  *noise_level_man,
                             Int32  *noise_level_exp,
                             Int32  *nrg_tone_man,
                             Int32  *nrg_tone_exp,
                             Int32  band_nrg_tone_detector,
                             const Int32 *frame_info,
                             Int32  *harm_index,
                             Int32  *phase_index,
                             Int32  i,
                             Int32  lowSubband,
                             Int32  noSubbands,
                             Int32  noNoiseFlag)
{

    Int32 *ptrReal;
    Int32 sb_gain_man;
    Int32 sb_noise_man;
    Int32 sb_noise_exp;
    Int32 l;
    Int32 k;
    Int32 tmp_q1;
    Int32 tmp_q2;
    Int32 tone_count;
    Int16 tmp_16;
    Int32 indexMinus1;
    Int32 indexPlus1;

    if (band_nrg_tone_detector)
    {

        for (k = 0; k < noSubbands; k++)
        {
            tmp_q2 = (-nrg_tone_exp[k]);
            tmp_q1 = nrg_tone_man[k];
            tmp_q2 = tmp_q1 >> tmp_q2;
            tmp_q1 = fxp_mul32_by_16(tmp_q2, Q15fmt(0.0163f));
            nrg_tone_man[k] = tmp_q2;
            nrg_tone_exp[k] = tmp_q1;
            noise_level_exp[k] += 1;
            nrg_gain_exp[k] += 28;
        }

        for (l = (frame_info[1+i] << 1); l < (frame_info[2+i] << 1); l++)
        {
            ptrReal = (aBufR + l * SBR_NUM_BANDS);

            tone_count = 0;

            indexPlus1  = (*harm_index + 1) & 3;

            if (indexPlus1 & 1)
            {
                for (k = 0; k < noSubbands; k++)
                {

                    sb_gain_man = nrg_gain_man[k];
                    tmp_q1 = *ptrReal;
                    tmp_q2 = nrg_gain_exp[k];
                    tmp_q1 = fxp_mul32_Q28(tmp_q1, sb_gain_man);

                    if (tmp_q2 < 0)
                    {
                        if (tmp_q2 > -32)
                        {
                            *ptrReal = tmp_q1 >> (-tmp_q2);
                        }
                    }
                    else
                    {
                        *ptrReal = tmp_q1 << tmp_q2;
                    }

                    *phase_index = (*phase_index + 1) & 511;

                    if (!nrg_tone_man[k] && !noNoiseFlag)

                    {
                        tmp_16 = rP_LCx[*phase_index];
                        sb_noise_man = noise_level_man[k];
                        sb_noise_exp = noise_level_exp[k];

                        tmp_q1 = fxp_mul32_by_16(sb_noise_man, tmp_16);

                        if (sb_noise_exp < 0)
                        {
                            if (sb_noise_exp > -32)
                            {
                                *ptrReal += tmp_q1 >> (-sb_noise_exp);
                            }
                        }
                        else
                        {
                            *ptrReal += tmp_q1 << sb_noise_exp;
                        }
                    }

                    tmp_q1 = nrg_tone_man[k];

                    if (*harm_index)
                    {
                        *ptrReal -= tmp_q1;
                    }
                    else
                    {
                        *ptrReal += tmp_q1;
                    }

                    if (tmp_q1)
                    {
                        tone_count++;
                    }

                    ptrReal++;

                }

            }
            else
            {
                indexMinus1 = (*harm_index - 1) & 3;

                sb_gain_man = nrg_gain_man[0];
                tmp_q1 = *ptrReal;
                tmp_q2 = nrg_gain_exp[0];
                tmp_q1 = fxp_mul32_Q28(tmp_q1, sb_gain_man);

                if (tmp_q2 < 0)
                {
                    if (tmp_q2 > -32)
                    {
                        *ptrReal = tmp_q1 >> (-tmp_q2);
                    }
                }
                else
                {
                    *ptrReal = tmp_q1 << tmp_q2;
                }

                *phase_index = (*phase_index + 1) & 511;

                tmp_q1 = nrg_tone_exp[0];
                tmp_q2 = nrg_tone_exp[1];

                if ((indexPlus1 != 0) ^((lowSubband & 1) != 0))
                {
                    *(ptrReal - 1) -= tmp_q1;
                    *(ptrReal)   += tmp_q2;
                }
                else
                {
                    *(ptrReal - 1) += tmp_q1;
                    *(ptrReal)   -= tmp_q2;
                }

                if (!nrg_tone_man[0] && !noNoiseFlag)
                {
                    tmp_16 = rP_LCx[*phase_index];
                    sb_noise_man = noise_level_man[0];
                    sb_noise_exp = noise_level_exp[0];

                    tmp_q1 = fxp_mul32_by_16(sb_noise_man, tmp_16);

                    if (sb_noise_exp < 0)
                    {
                        if (sb_noise_exp > -32)
                        {
                            *ptrReal += tmp_q1 >> (-sb_noise_exp);
                        }
                    }
                    else
                    {
                        *ptrReal += tmp_q1 << sb_noise_exp;
                    }
                }
                else
                {
                    tone_count++;
                }

                ptrReal++;

                for (k = 1; k < noSubbands - 1; k++)
                {

                    sb_gain_man = nrg_gain_man[k];
                    tmp_q1 = *ptrReal;
                    tmp_q2 = nrg_gain_exp[k];
                    tmp_q1 = fxp_mul32_Q28(tmp_q1, sb_gain_man);

                    if (tmp_q2 < 0)
                    {
                        if (tmp_q2 > -32)
                        {
                            *ptrReal = tmp_q1 >> (-tmp_q2);
                        }
                    }
                    else
                    {
                        *ptrReal = tmp_q1 << tmp_q2;
                    }

                    *phase_index = (*phase_index + 1) & 511;

                    if (tone_count < 16)
                    {
                        tmp_q1 = nrg_tone_exp[k - 1];
                        tmp_q2 = nrg_tone_exp[k + 1];

                        tmp_q1 -= tmp_q2;

                        if ((indexPlus1 != 0) ^(((k + lowSubband) & 1) != 0))
                        {
                            *(ptrReal) -= tmp_q1;
                        }
                        else
                        {
                            *(ptrReal) += tmp_q1;
                        }
                    }

                    if (!nrg_tone_man[k] && !noNoiseFlag)
                    {
                        tmp_16 = rP_LCx[*phase_index];
                        sb_noise_man = noise_level_man[k];
                        sb_noise_exp = noise_level_exp[k];

                        tmp_q1 = fxp_mul32_by_16(sb_noise_man, tmp_16);

                        if (sb_noise_exp < 0)
                        {
                            if (sb_noise_exp > -32)
                            {
                                *ptrReal += tmp_q1 >> (-sb_noise_exp);
                            }
                        }
                        else
                        {
                            *ptrReal += tmp_q1 << sb_noise_exp;
                        }
                    }
                    else
                    {
                        tone_count++;
                    }

                    ptrReal++;

                }

                sb_gain_man = nrg_gain_man[k];
                tmp_q1 = *ptrReal;
                tmp_q2 = nrg_gain_exp[k];
                tmp_q1 = fxp_mul32_Q28(tmp_q1, sb_gain_man);

                if (tmp_q2 < 0)
                {
                    if (tmp_q2 > -31)
                    {
                        *ptrReal = tmp_q1 >> (-tmp_q2);
                    }
                }
                else
                {
                    *ptrReal = tmp_q1 << tmp_q2;
                }

                *phase_index = (*phase_index + 1) & 511;

                if ((tone_count < 16) && !(indexMinus1 &1))
                {
                    tmp_q1 = nrg_tone_exp[k - 1];
                    tmp_q2 = nrg_tone_exp[k    ];

                    if ((indexMinus1 != 0) ^(((k + lowSubband) & 1) != 0))
                    {
                        *(ptrReal)   += tmp_q1;

                        if (k + lowSubband < 62)
                        {
                            *(ptrReal + 1) -= tmp_q2;
                        }
                    }
                    else
                    {
                        *(ptrReal)   -= tmp_q1;

                        if (k + lowSubband < 62)
                        {
                            *(ptrReal + 1) += tmp_q2;
                        }
                    }
                }

                if (!nrg_tone_man[k] && !noNoiseFlag)
                {
                    tmp_16 = rP_LCx[*phase_index];
                    sb_noise_man = noise_level_man[k];
                    sb_noise_exp = noise_level_exp[k];

                    tmp_q1 = fxp_mul32_by_16(sb_noise_man, tmp_16);

                    if (sb_noise_exp < 0)
                    {
                        if (sb_noise_exp > -31)
                        {
                            *ptrReal += tmp_q1 >> (-sb_noise_exp);
                        }
                    }
                    else
                    {
                        *ptrReal += tmp_q1 << sb_noise_exp;
                    }
                }

            }

            *harm_index = indexPlus1;

        }

    }
    else
    {

        for (k = 0; k < noSubbands; k++)
        {
            tmp_q1 = noise_level_exp[k];
            tmp_q2 = nrg_gain_exp[k];
            noise_level_exp[k] =  tmp_q1 + 1;
            nrg_gain_exp[k] = tmp_q2 + 28;
        }

        for (l = (frame_info[1+i] << 1); l < (frame_info[2+i] << 1); l++)
        {
            ptrReal = (aBufR + l * SBR_NUM_BANDS);

            for (k = 0; k < noSubbands; k++)
            {

                tmp_q1 = *ptrReal;
                sb_gain_man = nrg_gain_man[k];

                tmp_q2 = nrg_gain_exp[k];

                tmp_q1 = fxp_mul32_Q28(tmp_q1, sb_gain_man);

                if (tmp_q2 < 0)
                {
                    if (tmp_q2 > -31)
                    {
                        *ptrReal = tmp_q1 >> (-tmp_q2);
                    }
                }
                else
                {
                    *ptrReal = tmp_q1 << tmp_q2;
                }

                *phase_index = (*phase_index + 1) & 511;

                if (! noNoiseFlag)
                {
                    tmp_16 = rP_LCx[*phase_index];
                    sb_noise_man = noise_level_man[k];
                    sb_noise_exp = noise_level_exp[k];

                    tmp_q1 = fxp_mul32_by_16(sb_noise_man, tmp_16);

                    if (sb_noise_exp < 0)
                    {
                        if (sb_noise_exp > -31)
                        {
                            *ptrReal += tmp_q1 >> (-sb_noise_exp);
                        }
                    }
                    else
                    {
                        *ptrReal += tmp_q1 << sb_noise_exp;
                    }
                }

                ptrReal++;

            }

            *harm_index  = (*harm_index + 1) & 3;

        }

    }

}

#define Qfmt15(a)   (Int32)(a*((Int32)1<<15) + (a>=0?0.5F:-0.5F))

const Int16 pow2[39] = { 0, 0, 1, 0, 2,
                         0, Qfmt15(2 / 6.0f), 0, 3, 0, Qfmt15(2 / 10.0f), 0, Qfmt15(2 / 12.0f), 0, Qfmt15(2 / 14.0f), 0, 4,
                         0, Qfmt15(2 / 18.0f),    0, Qfmt15(2 / 20.0f), 0, Qfmt15(2 / 22.0f), 0, Qfmt15(2 / 24.0f),
                         0, Qfmt15(2 / 26.0f), 0, Qfmt15(2 / 28.0f), 0, Qfmt15(2 / 30.0f), 0, 5, 0, Qfmt15(2 / 34.0f),
                         0, Qfmt15(2 / 36.0f), 0, Qfmt15(2 / 38.0f)
                       };

void energy_estimation_LC(Int32 *aBufR,
                          Int32 *nrg_est_man,
                          Int32 *nrg_est_exp,
                          const Int32 *frame_info,
                          Int32 i,
                          Int32 k,
                          Int32 c,
                          Int32 ui2)
{

    Int32  aux1;
    Int32  aux2;
    Int32  l;

    Int64 nrg_h = 0;
    Int32 tmp1;
    UInt32 tmp2;

    for (l = ui2; l < (frame_info[2+i] << 1); l++)
    {

        aux1 = aBufR[l++*SBR_NUM_BANDS + k ];
        aux2 = aBufR[l  *SBR_NUM_BANDS + k ];

        nrg_h = fxp_mac64_Q31(nrg_h, aux1, aux1);
        nrg_h = fxp_mac64_Q31(nrg_h, aux2, aux2);
    }

    if (nrg_h < 0)
    {
        nrg_h = 0x7fffffff;
    }

    if (nrg_h)
    {
        tmp2 = (UInt32)(nrg_h >> 32);
        if (tmp2)
        {
            aux2 = pv_normalize(tmp2);
            aux2 -= 1;
            nrg_h = (nrg_h << aux2) >> 33;
            tmp2 = (UInt32)(nrg_h);
            nrg_est_exp[c] = 33 - aux2;
        }
        else
        {
            tmp2 = (UInt32)(nrg_h >> 2);
            aux2 = pv_normalize(tmp2);
            aux2 -= 1;

            tmp2 = (tmp2 << aux2);
            nrg_est_exp[c] =  -aux2 + 2;
        }

        tmp1 = (l - ui2);

        aux2 = pow2[tmp1];
        if (tmp1 == (tmp1 & (-tmp1)))
        {
            nrg_est_man[c] = tmp2 >> aux2;
        }
        else
        {
            nrg_est_man[c] = fxp_mul32_by_16(tmp2, aux2);
        }

    }
    else
    {
        nrg_est_man[c] = 0;
        nrg_est_exp[c] = -100;
    }

}

#if HQ_SBR

void envelope_application(Int32  *aBufR,
                          Int32  *aBufI,
                          Int32  *nrg_gain_man,
                          Int32  *nrg_gain_exp,
                          Int32  *noise_level_man,
                          Int32  *noise_level_exp,
                          Int32  *nrg_tone_man,
                          Int32  *nrg_tone_exp,
                          Int32  *fBuf_man[64],
                          Int32  *fBuf_exp[64],
                          Int32  *fBufN_man[64],
                          Int32  *fBufN_exp[64],
                          const  Int32 *frame_info,
                          Int32  *harm_index,
                          Int32  *phase_index,
                          Int32  i,
                          Int32  lowSubband,
                          Int32  noSubbands,
                          Int32  noNoiseFlag,
                          Int32  band_nrg_tone_detector,
                          Int32  maxSmoothLength,
                          Int32  smooth_length)
{

    Int32 *ptrReal;
    Int32 *ptrImag;
    Int32 sb_gain_man;
    Int32 sb_gain_exp;
    Int32 sb_noise_man;
    Int32 sb_noise_exp;
    Int32 l;
    Int32 k;
    Int32 n;
    Int32 tmp_q1;
    Int32 tmp_q2;
    Int32  aux1;
    Int32  aux2;
    Int32  filter_history = 0;

    if (band_nrg_tone_detector)
    {

        ptrReal = nrg_tone_exp;
        ptrImag = nrg_tone_man;
        tmp_q1 = - *(ptrReal++);
        aux1   =   *(ptrImag);
        for (k = 0; k < noSubbands; k++)
        {
            *(ptrImag++) = aux1 >> tmp_q1;
            tmp_q1 = - *(ptrReal++);
            aux1   =   *(ptrImag);
        }

        for (l = (frame_info[1+i] << 1); l < (frame_info[2+i] << 1); l++)
        {
            ptrReal = (aBufR + l * SBR_NUM_BANDS);
            ptrImag = (aBufI + l * SBR_NUM_BANDS);

            if (filter_history <= maxSmoothLength)
            {
                pv_memmove(fBuf_man[maxSmoothLength], nrg_gain_man, noSubbands*sizeof(*nrg_gain_man));
                pv_memmove(fBuf_exp[maxSmoothLength], nrg_gain_exp, noSubbands*sizeof(*nrg_gain_exp));
                pv_memmove(fBufN_man[maxSmoothLength], noise_level_man, noSubbands*sizeof(*noise_level_man));
                pv_memmove(fBufN_exp[maxSmoothLength], noise_level_exp, noSubbands*sizeof(*noise_level_exp));
            }

            for (k = 0; k < noSubbands; k++)
            {
                if (smooth_length == 0)
                {
                    sb_gain_man = nrg_gain_man[k];
                    sb_gain_exp = nrg_gain_exp[k];

                    sb_noise_man = noise_level_man[k];
                    sb_noise_exp = noise_level_exp[k];

                }
                else
                {

                    sb_gain_exp = fBuf_exp[maxSmoothLength][k];

                    sb_noise_exp = fBufN_exp[maxSmoothLength][k];

                    for (n = maxSmoothLength - smooth_length; n < maxSmoothLength; n++)
                    {
                        if (sb_gain_exp  < fBuf_exp[n][k])
                        {
                            sb_gain_exp = fBuf_exp[n][k];
                        }

                        if (sb_noise_exp  < fBufN_exp[n][k])
                        {
                            sb_noise_exp = fBufN_exp[n][k];
                        }
                    }

                    sb_gain_man = fxp_mul32_Q30(fBuf_man[maxSmoothLength][k], Q30fmt(0.33333333333333f));
                    sb_gain_man  = sb_gain_man >> (sb_gain_exp - fBuf_exp[maxSmoothLength][k]);

                    sb_noise_man = fxp_mul32_Q30(fBufN_man[maxSmoothLength][k], Q30fmt(0.33333333333333f));
                    sb_noise_man = sb_noise_man >> (sb_noise_exp - fBufN_exp[maxSmoothLength][k]);

                    n = maxSmoothLength - smooth_length;

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.03183050093751f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.03183050093751f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.11516383427084f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.11516383427084f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.21816949906249f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.21816949906249f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.30150283239582f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.30150283239582f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n][k]);

                }

                aux1 = *ptrReal;
                aux2 = *ptrImag;
                sb_gain_exp += 32;
                aux1 = fxp_mul32_Q31(aux1, sb_gain_man);
                aux2 = fxp_mul32_Q31(aux2, sb_gain_man);

                if (sb_gain_exp < 0)
                {
                    sb_gain_exp = -sb_gain_exp;
                    if (sb_gain_exp < 32)
                    {
                        *ptrReal = (aux1 >> sb_gain_exp);
                        *ptrImag = (aux2 >> sb_gain_exp);
                    }
                }
                else
                {
                    *ptrReal = (aux1 << sb_gain_exp);
                    *ptrImag = (aux2 << sb_gain_exp);
                }

                *phase_index = (*phase_index + 1) & 511;

                if (nrg_tone_man[k] || noNoiseFlag)
                {
                    sb_noise_man = 0;
                    sb_noise_exp = 0;
                }
                else
                {

                    Int32 tmp = rPxx[*phase_index];
                    sb_noise_exp += 1;
                    tmp_q1 = fxp_mul32_by_16t(sb_noise_man, tmp);
                    tmp_q2 = fxp_mul32_by_16b(sb_noise_man, tmp);

                    if (sb_noise_exp < 0)
                    {
                        if (sb_noise_exp > -32)
                        {
                            *ptrReal += tmp_q1 >> (-sb_noise_exp);
                            *ptrImag += tmp_q2 >> (-sb_noise_exp);
                        }
                    }
                    else
                    {
                        *ptrReal += tmp_q1 << sb_noise_exp;
                        *ptrImag += tmp_q2 << sb_noise_exp;
                    }
                }

                tmp_q1 = nrg_tone_man[k];

                if (*harm_index & 1)
                {
                    if ((((k + lowSubband) & 1) != 0) ^(*harm_index != 1))
                    {
                        *ptrImag  -=  tmp_q1;
                    }
                    else
                    {
                        *ptrImag  +=  tmp_q1;
                    }
                }
                else
                {
                    *ptrReal += (*harm_index) ? -tmp_q1 : tmp_q1;
                }

                *ptrReal++ <<= 10;
                *ptrImag++ <<= 10;

            }

            *harm_index = (*harm_index + 1) & 3;

            if (filter_history++ < maxSmoothLength)
            {

                ptrReal = (Int32 *)fBuf_man[0];
                ptrImag = (Int32 *)fBufN_man[0];

                for (n = 0; n < maxSmoothLength; n++)
                {
                    fBuf_man[n]  = fBuf_man[n+1];
                    fBufN_man[n] = fBufN_man[n+1];
                }

                fBuf_man[maxSmoothLength]  = ptrReal;
                fBufN_man[maxSmoothLength] = ptrImag;

                ptrReal = (Int32 *)fBuf_exp[0];
                ptrImag = (Int32 *)fBufN_exp[0];

                for (n = 0; n < maxSmoothLength; n++)
                {
                    fBuf_exp[n]  = fBuf_exp[n+1];
                    fBufN_exp[n] = fBufN_exp[n+1];
                }

                fBuf_exp[maxSmoothLength]  = ptrReal;
                fBufN_exp[maxSmoothLength] = ptrImag;
            }

        }

    }
    else
    {

        for (l = (frame_info[1+i] << 1); l < (frame_info[2+i] << 1); l++)
        {
            ptrReal = (aBufR + l * SBR_NUM_BANDS);
            ptrImag = (aBufI + l * SBR_NUM_BANDS);

            if (filter_history <= maxSmoothLength)
            {
                pv_memmove(fBuf_man[maxSmoothLength], nrg_gain_man, noSubbands*sizeof(*nrg_gain_man));
                pv_memmove(fBuf_exp[maxSmoothLength], nrg_gain_exp, noSubbands*sizeof(*nrg_gain_exp));
                pv_memmove(fBufN_man[maxSmoothLength], noise_level_man, noSubbands*sizeof(*noise_level_man));
                pv_memmove(fBufN_exp[maxSmoothLength], noise_level_exp, noSubbands*sizeof(*noise_level_exp));
            }

            for (k = 0; k < noSubbands; k++)
            {
                if (smooth_length == 0)
                {
                    sb_gain_man = nrg_gain_man[k];
                    sb_gain_exp = nrg_gain_exp[k];

                    sb_noise_man = noise_level_man[k];
                    sb_noise_exp = noise_level_exp[k];

                }
                else
                {

                    sb_gain_exp = fBuf_exp[maxSmoothLength][k];

                    sb_noise_exp = fBufN_exp[maxSmoothLength][k];

                    for (n = maxSmoothLength - smooth_length; n < maxSmoothLength; n++)
                    {
                        if (sb_gain_exp  < fBuf_exp[n][k])
                        {
                            sb_gain_exp = fBuf_exp[n][k];
                        }

                        if (sb_noise_exp  < fBufN_exp[n][k])
                        {
                            sb_noise_exp = fBufN_exp[n][k];
                        }
                    }

                    sb_gain_man = fxp_mul32_Q30(fBuf_man[maxSmoothLength][k], Q30fmt(0.33333333333333f));
                    sb_gain_man  = sb_gain_man >> (sb_gain_exp - fBuf_exp[maxSmoothLength][k]);

                    sb_noise_man = fxp_mul32_Q30(fBufN_man[maxSmoothLength][k], Q30fmt(0.33333333333333f));
                    sb_noise_man = sb_noise_man >> (sb_noise_exp - fBufN_exp[maxSmoothLength][k]);

                    n = maxSmoothLength - smooth_length;

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.03183050093751f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.03183050093751f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.11516383427084f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.11516383427084f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.21816949906249f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.21816949906249f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n++][k]);

                    tmp_q1 = fxp_mul32_Q30(fBuf_man[n][k], Q30fmt(0.30150283239582f));
                    sb_gain_man  += tmp_q1 >> (sb_gain_exp - fBuf_exp[n][k]);

                    tmp_q1 = fxp_mul32_Q30(fBufN_man[n][k], Q30fmt(0.30150283239582f));
                    sb_noise_man += tmp_q1 >> (sb_noise_exp - fBufN_exp[n][k]);

                }

                aux1 = *ptrReal;
                aux2 = *ptrImag;
                sb_gain_exp += 32;
                aux1 = fxp_mul32_Q31(aux1, sb_gain_man);
                aux2 = fxp_mul32_Q31(aux2, sb_gain_man);

                if (sb_gain_exp < 0)
                {
                    if (sb_gain_exp > -32)
                    {
                        if (sb_gain_exp > -10)
                        {
                            *ptrReal = aux1 << (10 + sb_gain_exp);
                            *ptrImag = aux2 << (10 + sb_gain_exp);
                        }
                        else
                        {
                            *ptrReal = aux1 >> (-sb_gain_exp - 10);
                            *ptrImag = aux2 >> (-sb_gain_exp - 10);
                        }
                    }
                }
                else
                {
                    *ptrReal = aux1 << (sb_gain_exp + 10);
                    *ptrImag = aux2 << (sb_gain_exp + 10);
                }

                *phase_index = (*phase_index + 1) & 511;

                if (!noNoiseFlag)
                {

                    Int32 tmp = rPxx[*phase_index];
                    sb_noise_exp += 1;
                    tmp_q1 = fxp_mul32_by_16t(sb_noise_man, tmp);
                    tmp_q2 = fxp_mul32_by_16b(sb_noise_man, tmp);

                    if (sb_noise_exp < 0)
                    {
                        if (sb_noise_exp > -32)
                        {
                            if (sb_noise_exp > -10)
                            {
                                *ptrReal += tmp_q1 << (10 + sb_noise_exp);
                                *ptrImag += tmp_q2 << (10 + sb_noise_exp);
                            }
                            else
                            {
                                *ptrReal += tmp_q1 >> (-sb_noise_exp - 10);
                                *ptrImag += tmp_q2 >> (-sb_noise_exp - 10);
                            }
                        }
                    }
                    else
                    {
                        *ptrReal += tmp_q1 << (sb_noise_exp + 10);
                        *ptrImag += tmp_q2 << (sb_noise_exp + 10);
                    }
                }

                ptrReal++;
                ptrImag++;

            }

            *harm_index = (*harm_index + 1) & 3;

            if (filter_history++ < maxSmoothLength)
            {

                ptrReal = (Int32 *)fBuf_man[0];
                ptrImag = (Int32 *)fBufN_man[0];

                for (n = 0; n < maxSmoothLength; n++)
                {
                    fBuf_man[n]  = fBuf_man[n+1];
                    fBufN_man[n] = fBufN_man[n+1];
                }

                fBuf_man[maxSmoothLength]  = ptrReal;
                fBufN_man[maxSmoothLength] = ptrImag;

                ptrReal = (Int32 *)fBuf_exp[0];
                ptrImag = (Int32 *)fBufN_exp[0];

                for (n = 0; n < maxSmoothLength; n++)
                {
                    fBuf_exp[n]  = fBuf_exp[n+1];
                    fBufN_exp[n] = fBufN_exp[n+1];
                }

                fBuf_exp[maxSmoothLength]  = ptrReal;
                fBufN_exp[maxSmoothLength] = ptrImag;
            }

        }

    }

}

void energy_estimation(Int32 *aBufR,
                       Int32 *aBufI,
                       Int32 *nrg_est_man,
                       Int32 *nrg_est_exp,
                       const Int32 *frame_info,
                       Int32 i,
                       Int32 k,
                       Int32 c,
                       Int32 ui2)
{

    Int32  aux1;
    Int32  aux2;
    Int32  l;

    Int64 nrg_h = 0;
    Int32 tmp1;
    Int32 tmp2;

    aux1 = aBufR[ui2*SBR_NUM_BANDS + k];
    aux2 = aBufI[ui2*SBR_NUM_BANDS + k];
    for (l = ui2 + 1; l < (frame_info[2+i] << 1);  l++)
    {
        nrg_h = fxp_mac64_Q31(nrg_h, aux1, aux1);
        nrg_h = fxp_mac64_Q31(nrg_h, aux2, aux2);
        aux1 = aBufR[l*SBR_NUM_BANDS + k];
        aux2 = aBufI[l*SBR_NUM_BANDS + k];
    }
    nrg_h = fxp_mac64_Q31(nrg_h, aux1, aux1);
    nrg_h = fxp_mac64_Q31(nrg_h, aux2, aux2);

    if (nrg_h < 0)
    {
        nrg_h = 0x7fffffff;
    }

    if (nrg_h)
    {

        aux1 = (UInt32)(nrg_h >> 32);
        if (aux1)
        {
            aux2 = pv_normalize(aux1);
            if (aux2)
            {
                aux2 -= 1;
                nrg_h = (nrg_h << aux2) >> 33;
                tmp2 = (UInt32)(nrg_h);
                nrg_est_exp[c] = 33 - aux2;
            }
            else
            {
                tmp2 = (UInt32)(aux1 >> 1);
                nrg_est_exp[c] = 33 ;

            }
        }
        else
        {
            aux1 = (UInt32)(nrg_h >> 1);
            aux2 = pv_normalize(aux1);

            tmp2 = (aux1 << aux2);
            nrg_est_exp[c] =  -aux2 + 1;

        }

        tmp1 = (l - ui2);
        aux2 = pow2[tmp1];
        if (tmp1 == (tmp1 & (-tmp1)))
        {
            nrg_est_man[c] = tmp2 >> aux2;
        }
        else
        {
            nrg_est_man[c] = fxp_mul32_by_16(tmp2, aux2);
        }
    }
    else
    {
        nrg_est_man[c] = 0;
        nrg_est_exp[c] = -100;
    }

}

#endif

#endif

