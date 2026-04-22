

#include    "config.h"
#ifdef AAC_PLUS

#include    "sbr_generate_high_freq.h"
#include    "calc_auto_corr.h"
#include    "sbr_inv_filt_levelemphasis.h"
#include    "pv_div.h"
#include    "fxp_mul32.h"
#include    "aac_mem_funcs.h"
#include    "sbr_constants.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void high_freq_coeff_LC(Int32 sourceBufferReal[][32],
    Int32 *alphar[2],
    Int32 *degreeAlias,
    Int32 *v_k_master,
    Int32 *scratch_mem);

    void high_freq_generation_LC(Int32 sourceBufferReal[][32],
                                 Int32 *targetBufferReal,
                                 Int32 *alphar[2],
                                 Int32 *degreeAlias,
                                 Int32 *invFiltBandTable,
                                 Int32 targetStopBand,
                                 Int32 patchDistance,
                                 Int32 numBandsInPatch,
                                 Int32 startSample,
                                 Int32 slopeLength,
                                 Int32 stopSample,
                                 Int32 *BwVector,
                                 Int32 sbrStartFreqOffset);

#ifdef HQ_SBR

    void high_freq_coeff(Int32 sourceBufferReal[][32],
                         Int32 sourceBufferImag[][32],
                         Int32 *alphar[2],
                         Int32 *alphai[2],
                         Int32 *v_k_master);

    void high_freq_generation(Int32 sourceBufferReal[][32],
                              Int32 sourceBufferImag[][32],
                              Int32 *targetBufferReal,
                              Int32 *targetBufferImag,
                              Int32 *alphar[2],
                              Int32 *alphai[2],
                              Int32 *invFiltBandTable,
                              Int32 targetStopBand,
                              Int32 patchDistance,
                              Int32 numBandsInPatch,
                              Int32 startSample,
                              Int32 slopeLength,
                              Int32 stopSample,
                              Int32 *BwVector,
                              Int32 sbrStartFreqOffset);

#endif

#ifdef __cplusplus
}
#endif

void sbr_generate_high_freq(Int32 sourceBufferReal[][32],
                            Int32 sourceBufferImag[][32],
                            Int32 *targetBufferReal,
                            Int32 *targetBufferImag,
                            INVF_MODE *invFiltMode,
                            INVF_MODE *prevInvFiltMode,
                            Int32 *invFiltBandTable,
                            Int32 noInvFiltBands,
                            Int32 highBandStartSb,
                            Int32  *v_k_master,
                            Int32 numMaster,
                            Int32 fs,
                            Int32   *frameInfo,
                            Int32 *degreeAlias,
                            Int32  scratch_mem[][64],
                            Int32  BwVector[MAX_NUM_PATCHES],
                            Int32  BwVectorOld[MAX_NUM_PATCHES],
                            struct PATCH *Patch,
                            Int32 LC_flag,
                            Int32 *highBandStopSb)
{
    Int32    i;
    Int32    patch;
    Int32    startSample;
    Int32    stopSample;
    Int32    goalSb;
    Int32    targetStopBand;
    Int32    sourceStartBand;
    Int32    patchDistance;
    Int32    numBandsInPatch;
    Int32    sbrStartFreqOffset;

    Int32  *alphar[2];
    Int32  *alphai[2];

    Int32    lsb = v_k_master[0];
    Int32    usb = v_k_master[numMaster];
    Int32  xoverOffset = highBandStartSb - v_k_master[0];

    Int    slopeLength = 0;

    Int32 firstSlotOffs = frameInfo[1];
    Int32 lastSlotOffs  = frameInfo[frameInfo[0] + 1] - 16;

    alphar[0] = scratch_mem[0];
    alphar[1] = scratch_mem[1];
    alphai[0] = scratch_mem[2];
    alphai[1] = scratch_mem[3];

    startSample = (firstSlotOffs << 1);
    stopSample  = (lastSlotOffs << 1) + 32;

    sbr_inv_filt_levelemphasis(invFiltMode,
                               prevInvFiltMode,
                               noInvFiltBands,
                               BwVector,
                               BwVectorOld);

    if (LC_flag == ON)
    {

        pv_memset((void *)&targetBufferReal[startSample*SBR_NUM_BANDS],
                  0,
                  (stopSample - startSample)*SBR_NUM_BANDS*sizeof(targetBufferReal[0]));

        high_freq_coeff_LC(sourceBufferReal,
                           alphar,
                           degreeAlias,
                           v_k_master,
                           scratch_mem[4]);
    }
#ifdef HQ_SBR
    else
    {

        pv_memset((void *)&targetBufferReal[startSample*SBR_NUM_BANDS],
                  0,
                  (stopSample - startSample)*SBR_NUM_BANDS*sizeof(targetBufferReal[0]));
        pv_memset((void *)&targetBufferImag[startSample*SBR_NUM_BANDS],
                  0,
                  (stopSample - startSample)*SBR_NUM_BANDS*sizeof(targetBufferImag[0]));

        high_freq_coeff(sourceBufferReal,
                        sourceBufferImag,
                        alphar,
                        alphai,
                        v_k_master);

    }
#endif

    switch (fs)

    {

        case 48000:
            goalSb = 43;
            break;
        case 32000:
            goalSb = 64;
            break;
        case 24000:
            goalSb = 85;
            break;
        case 22050:
            goalSb = 93;
            break;
        case 16000:
            goalSb = 128;
            break;
        case 44100:
        default:
            goalSb = 46;
            break;
    }

    i = 0;

    if (goalSb > v_k_master[0])
    {
        if (goalSb < v_k_master[numMaster])
        {
            while (v_k_master[i] < goalSb)
            {
                i++;
            }
        }
        else
        {
            i = numMaster;
        }
    }

    goalSb =  v_k_master[i];

    sourceStartBand = xoverOffset + 1;
    targetStopBand = lsb + xoverOffset;

    patch = 0;

    sbrStartFreqOffset = targetStopBand;

    while (targetStopBand < usb)
    {
        Patch->targetStartBand[patch] = targetStopBand;

        numBandsInPatch = goalSb - targetStopBand;

        if (numBandsInPatch >= lsb - sourceStartBand)
        {

            patchDistance   = targetStopBand - sourceStartBand;
            patchDistance   = patchDistance & ~1;
            numBandsInPatch = lsb - (targetStopBand - patchDistance);

            if (targetStopBand + numBandsInPatch > v_k_master[0])
            {
                i = numMaster;
                if (targetStopBand + numBandsInPatch < v_k_master[numMaster])
                {
                    while (v_k_master[i] > targetStopBand + numBandsInPatch)
                    {
                        i--;
                    }
                }
            }
            else
            {
                i = 0;
            }
            numBandsInPatch =  v_k_master[i] - targetStopBand;
        }

        patchDistance   = numBandsInPatch + targetStopBand - lsb;
        patchDistance   = (patchDistance + 1) & ~1;

        sourceStartBand = 1;

        if (goalSb - (targetStopBand + numBandsInPatch) < 3)
        {
            goalSb = usb;
        }

        if ((numBandsInPatch < 3) && (patch > 0))
        {
            if (LC_flag == ON)
            {

                pv_memset((void *) &degreeAlias[targetStopBand], 0, numBandsInPatch*sizeof(*degreeAlias));
            }
            break;
        }

        if (numBandsInPatch <= 0)
        {
            continue;
        }

        if (LC_flag == ON)
        {

            high_freq_generation_LC(sourceBufferReal,
                                    (Int32 *)targetBufferReal,
                                    alphar,
                                    degreeAlias,
                                    invFiltBandTable,
                                    targetStopBand,
                                    patchDistance,
                                    numBandsInPatch,
                                    startSample,
                                    slopeLength,
                                    stopSample,
                                    BwVector,
                                    sbrStartFreqOffset);

        }
#ifdef HQ_SBR
        else
        {

            high_freq_generation(sourceBufferReal,
                                 sourceBufferImag,
                                 (Int32 *)targetBufferReal,
                                 (Int32 *)targetBufferImag,
                                 alphar,
                                 alphai,
                                 invFiltBandTable,
                                 targetStopBand,
                                 patchDistance,
                                 numBandsInPatch,
                                 startSample,
                                 slopeLength,
                                 stopSample,
                                 BwVector,
                                 sbrStartFreqOffset);

        }
#endif

        targetStopBand += numBandsInPatch;

        patch++;

    }

    Patch->noOfPatches = patch;

    pv_memmove(BwVectorOld, BwVector, noInvFiltBands*sizeof(BwVector[0]));

    *highBandStopSb = goalSb;

}

void high_freq_coeff_LC(Int32 sourceBufferReal[][32],
                        Int32 *alphar[2],
                        Int32 *degreeAlias,
                        Int32 *v_k_master,
                        Int32 *scratch_mem)
{

    Int32  fac;
    Int32 *k1;
    struct ACORR_COEFS ac;
    struct intg_div  quotient;

    Int32 temp1;
    Int32 temp2;
    Int32 temp3;
    Int32 autoCorrLength;
    Int32 loBand;

    k1 = scratch_mem;

    autoCorrLength = 38;

    for (loBand = 1; loBand < v_k_master[0]; loBand++)
    {

        calc_auto_corr_LC(&ac,
                          sourceBufferReal,
                          loBand,
                          autoCorrLength);

        if (ac.r11r && ac.det)
        {

            pv_div(ac.r01r, ac.r11r, &quotient);

            fac = -(quotient.quotient >> 2);

            if (quotient.shift_factor > 0)
            {
                fac >>= quotient.shift_factor;
            }
            else if (quotient.shift_factor < 0)
            {
                if (quotient.shift_factor > -4)
                {
                    fac <<= (-quotient.shift_factor);
                }
                else
                {
                    fac = 0x80000000;
                }
            }

            if (quotient.shift_factor > 0)
            {
                k1[loBand] = - quotient.quotient >> quotient.shift_factor;
            }
            else if (quotient.shift_factor == 0)
            {
                if (quotient.quotient >= 0x40000000)
                {
                    k1[loBand] = (Int32)0xC0000000;
                }
                else if (quotient.quotient <= (Int32)0xC0000000)
                {
                    k1[loBand] = 0x40000000;
                }
                else
                {
                    k1[loBand] = -quotient.quotient;
                }
            }
            else
            {
                if (quotient.quotient > 0)
                {
                    k1[loBand] = (Int32)0xC0000000;
                }
                else
                {
                    k1[loBand] = 0x40000000;
                }
            }

            temp1  = -fxp_mul32_Q30(ac.r02r, ac.r11r);
            temp1  =  fxp_mac32_Q30(ac.r01r, ac.r12r, temp1);

            temp2 = ac.det;
            temp3 = temp1 > 0 ? temp1 : -temp1;
            temp2 = temp2 > 0 ? temp2 : -temp2;

            if ((temp3 >> 2) >= temp2 || fac == (Int32)0x80000000)
            {
                alphar[0][loBand] = 0;
                alphar[1][loBand] = 0;
            }
            else
            {
                pv_div(temp1, ac.det, &quotient);

                alphar[1][loBand] = quotient.quotient;
                quotient.shift_factor += 2;

                if (quotient.shift_factor > 0)
                {
                    alphar[1][loBand] >>= quotient.shift_factor;
                }
                else if (quotient.shift_factor < 0)
                {
                    alphar[1][loBand] <<= (-quotient.shift_factor);
                }

                pv_div(ac.r12r, ac.r11r, &quotient);

                temp3 = (quotient.quotient >> 2);

                if (quotient.shift_factor > 0)
                {
                    temp3 >>= quotient.shift_factor;
                }
                else if (quotient.shift_factor < 0)
                {
                    temp3 <<= (-quotient.shift_factor);
                }

                alphar[0][loBand] = fac - fxp_mul32_Q28(alphar[1][loBand], temp3) ;

                if ((alphar[0][loBand] >= 0x40000000) || (alphar[0][loBand] <= (Int32)0xC0000000))
                {
                    alphar[0][loBand] = 0;
                    alphar[1][loBand] = 0;
                }

            }

        }
        else
        {
            alphar[0][loBand] = 0;
            alphar[1][loBand] = 0;

            k1[loBand] = 0;
        }

    }

    k1[0] = 0;
    degreeAlias[1] = 0;
    for (loBand = 2; loBand < v_k_master[0]; loBand++)
    {
        degreeAlias[loBand] = 0;
        if ((!(loBand & 1)) && (k1[loBand] < 0))
        {
            if (k1[loBand-1] < 0)
            {
                degreeAlias[loBand]   = 0x40000000;
                if (k1[loBand-2] > 0)
                {
                    degreeAlias[loBand-1] = 0x40000000 - fxp_mul32_Q30(k1[loBand-1], k1[loBand-1]);

                }
            }
            else if (k1[loBand-2] > 0)
            {
                degreeAlias[loBand] = 0x40000000 - fxp_mul32_Q30(k1[loBand-1], k1[loBand-1]);
            }
        }
        if ((loBand & 1) && (k1[loBand] > 0))
        {
            if (k1[loBand-1] > 0)
            {
                degreeAlias[loBand]   = 0x40000000;
                if (k1[loBand-2] < 0)
                {
                    degreeAlias[loBand-1] = 0x40000000 - fxp_mul32_Q30(k1[loBand-1], k1[loBand-1]);
                }
            }
            else if (k1[loBand-2] < 0)
            {
                degreeAlias[loBand] = 0x40000000 - fxp_mul32_Q30(k1[loBand-1], k1[loBand-1]);
            }
        }
    }

}

void high_freq_generation_LC(Int32 sourceBufferReal[][32],
                             Int32 *targetBufferReal,
                             Int32 *alphar[2],
                             Int32 *degreeAlias,
                             Int32 *invFiltBandTable,
                             Int32 targetStopBand,
                             Int32 patchDistance,
                             Int32 numBandsInPatch,
                             Int32 startSample,
                             Int32 slopeLength,
                             Int32 stopSample,
                             Int32 *BwVector,
                             Int32 sbrStartFreqOffset)
{

    Int32  temp1;
    Int32  temp2;
    Int32  temp3;

    Int32  a0r;
    Int32  a1r;
    Int32  i;
    Int32  bw;
    Int32  hiBand;
    Int32  bwIndex;
    Int32  loBand;
    Int32  j;

    bwIndex = 0;

    for (hiBand = targetStopBand; hiBand < targetStopBand + numBandsInPatch; hiBand++)
    {
        loBand = hiBand - patchDistance;

        if (hiBand != targetStopBand)
        {
            degreeAlias[hiBand] = degreeAlias[loBand];
        }
        else
        {
            degreeAlias[hiBand] = 0;
        }

        while (hiBand >= invFiltBandTable[bwIndex])
        {
            bwIndex++;
        }

        bw = BwVector[bwIndex];

        j = hiBand - sbrStartFreqOffset;

        if (bw > 0 && (alphar[0][loBand] | alphar[1][loBand]))
        {

            a0r = fxp_mul32_Q29(bw, alphar[0][loBand]);

            bw  = fxp_mul32_Q31(bw, bw) << 2;

            a1r = fxp_mul32_Q28(bw, alphar[1][loBand]);

            i = startSample + slopeLength;

            temp1 = sourceBufferReal[i    ][loBand];
            temp2 = sourceBufferReal[i - 1][loBand];
            temp3 = sourceBufferReal[i - 2][loBand];

            for (; i < stopSample + slopeLength - 1; i++)
            {

                targetBufferReal[i*SBR_NUM_BANDS + j] = temp1 + fxp_mul32_Q28(a0r, temp2)  +
                                                        fxp_mul32_Q28(a1r, temp3);

                temp3 = temp2;
                temp2 = temp1;
                temp1 = sourceBufferReal[i + 1][loBand];
            }
            targetBufferReal[i*SBR_NUM_BANDS + j] = temp1 + fxp_mul32_Q28(a0r, temp2)  +
                                                    fxp_mul32_Q28(a1r, temp3);

        }
        else
        {

            for (i = startSample + slopeLength; i < stopSample + slopeLength; i++)
            {
                targetBufferReal[i*SBR_NUM_BANDS + j] = sourceBufferReal[i][loBand];
            }
        }

    }

}

#ifdef HQ_SBR

void high_freq_coeff(Int32 sourceBufferReal[][32],
                     Int32 sourceBufferImag[][32],
                     Int32 *alphar[2],
                     Int32 *alphai[2],
                     Int32 *v_k_master)
{

    Int32  overflow_flag;

    Int32  temp1r;
    Int32  temp1i;
    Int32  temp0r;
    Int32  temp0i;
    Int32  loBand;

    struct ACORR_COEFS ac;
    struct intg_div  quotient;

    Int32 autoCorrLength;

    autoCorrLength = 38;

    for (loBand = 1; loBand < v_k_master[0]; loBand++)
    {

        calc_auto_corr(&ac,
                       sourceBufferReal,
                       sourceBufferImag,
                       loBand,
                       autoCorrLength);

        overflow_flag = 0;

        if (ac.det < 1)
        {

            temp1r = 0;
            temp1i = 0;
            alphar[1][loBand] = 0;
            alphai[1][loBand] = 0;

        }
        else
        {

            temp1r =  fxp_mul32_Q29(ac.r01r, ac.r12r);
            temp1r =  fxp_msu32_Q29(ac.r01i, ac.r12i, temp1r);
            temp1r =  fxp_msu32_Q29(ac.r02r, ac.r11r, temp1r);

            temp1i =  fxp_mul32_Q29(ac.r01r, ac.r12i);
            temp1i =  fxp_msu32_Q29(ac.r02i, ac.r11r, temp1i);
            temp1i =  fxp_mac32_Q29(ac.r01i, ac.r12r, temp1i);

            pv_div(temp1r, ac.det, &quotient);
            overflow_flag = (quotient.shift_factor < -2) ? 1 : 0;
            temp1r = quotient.quotient >> (2 + quotient.shift_factor);
            pv_div(temp1i, ac.det, &quotient);
            overflow_flag = (quotient.shift_factor < -2) ? 1 : 0;
            temp1i = quotient.quotient >> (2 + quotient.shift_factor);

            alphar[1][loBand] = temp1r;
            alphai[1][loBand] = temp1i;

        }

        if (ac.r11r == 0)
        {
            temp0r = 0;
            temp0i = 0;
            alphar[0][loBand] = 0;
            alphai[0][loBand] = 0;

        }
        else
        {
            temp0r = - (ac.r01r + fxp_mul32_Q28(temp1r, ac.r12r) + fxp_mul32_Q28(temp1i, ac.r12i));
            temp0i = - (ac.r01i + fxp_mul32_Q28(temp1i, ac.r12r) - fxp_mul32_Q28(temp1r, ac.r12i));

            pv_div(temp0r, ac.r11r, &quotient);
            overflow_flag = (quotient.shift_factor < -2) ? 1 : 0;
            temp0r = quotient.quotient >> (2 + quotient.shift_factor);
            pv_div(temp0i, ac.r11r, &quotient);
            overflow_flag = (quotient.shift_factor < -2) ? 1 : 0;
            temp0i = quotient.quotient >> (2 + quotient.shift_factor);

            alphar[0][loBand] = temp0r;
            alphai[0][loBand] = temp0i;

        }

        if (fxp_mul32_Q28((temp0r >> 2), (temp0r >> 2)) + fxp_mul32_Q28((temp0i >> 2), (temp0i >> 2)) >= 0x10000000 ||
                fxp_mul32_Q28((temp1r >> 2), (temp1r >> 2)) + fxp_mul32_Q28((temp1i >> 2), (temp1i >> 2)) >= 0x10000000 ||
                overflow_flag)

        {
            alphar[0][loBand] = 0;
            alphar[1][loBand] = 0;
            alphai[0][loBand] = 0;
            alphai[1][loBand] = 0;

        }
    }
}

void high_freq_generation(Int32 sourceBufferReal[][32],
                          Int32 sourceBufferImag[][32],
                          Int32 *targetBufferReal,
                          Int32 *targetBufferImag,
                          Int32 *alphar[2],
                          Int32 *alphai[2],
                          Int32 *invFiltBandTable,
                          Int32 targetStopBand,
                          Int32 patchDistance,
                          Int32 numBandsInPatch,
                          Int32 startSample,
                          Int32 slopeLength,
                          Int32 stopSample,
                          Int32 *BwVector,
                          Int32 sbrStartFreqOffset)
{
    Int32  temp1_r;
    Int32  temp2_r;
    Int32  temp3_r;
    Int32  temp1_i;
    Int32  temp2_i;
    Int32  temp3_i;

    Int32  a0i;
    Int32  a1i;
    Int32  a0r;
    Int32  a1r;
    Int32  i;
    Int32  bw;
    Int32  hiBand;
    Int32  bwIndex;
    Int32  loBand;
    Int32  j;

    Int64 tmp;

    bwIndex = 0;

    for (hiBand = targetStopBand; hiBand < targetStopBand + numBandsInPatch; hiBand++)
    {

        loBand = hiBand - patchDistance;

        while (hiBand >= invFiltBandTable[bwIndex])
        {
            bwIndex++;
        }

        bw = BwVector[bwIndex];

        j = hiBand - sbrStartFreqOffset;

        if (bw >= 0 && (alphar[0][loBand] | alphar[1][loBand] |
                        alphai[0][loBand] | alphai[1][loBand]))
        {

            a0r = fxp_mul32_Q29(bw, alphar[0][loBand]);
            a0i = fxp_mul32_Q29(bw, alphai[0][loBand]);

            bw  = fxp_mul32_Q30(bw, bw);

            a1r = fxp_mul32_Q28(bw, alphar[1][loBand]);
            a1i = fxp_mul32_Q28(bw, alphai[1][loBand]);

            i  = startSample + slopeLength;
            j += i * SBR_NUM_BANDS;

            temp1_r = sourceBufferReal[i    ][loBand];
            temp2_r = sourceBufferReal[i - 1][loBand];
            temp3_r = sourceBufferReal[i - 2][loBand];

            temp1_i = sourceBufferImag[i    ][loBand];
            temp2_i = sourceBufferImag[i - 1][loBand];
            temp3_i = sourceBufferImag[i - 2][loBand];

            while (i < stopSample + slopeLength)
            {
                tmp =  fxp_mac64_Q31(((Int64)temp1_r << 28),  a0r, temp2_r);
                tmp =  fxp_mac64_Q31(tmp, -a0i, temp2_i);
                tmp =  fxp_mac64_Q31(tmp,  a1r, temp3_r);
                targetBufferReal[j] = (Int32)(fxp_mac64_Q31(tmp, -a1i, temp3_i) >> 28);

                tmp =  fxp_mac64_Q31(((Int64)temp1_i << 28),  a0i, temp2_r);
                tmp =  fxp_mac64_Q31(tmp,  a0r, temp2_i);
                tmp =  fxp_mac64_Q31(tmp,  a1i, temp3_r);
                targetBufferImag[j] = (Int32)(fxp_mac64_Q31(tmp,  a1r, temp3_i) >> 28);

                i++;
                j += SBR_NUM_BANDS;

                temp3_r  = temp2_r;
                temp2_r  = temp1_r;
                temp1_r  = sourceBufferReal[i ][loBand];

                temp3_i  = temp2_i;
                temp2_i  = temp1_i;
                temp1_i  = sourceBufferImag[i ][loBand];

            }

        }

        else
        {
            i = startSample + slopeLength;
            j += i * SBR_NUM_BANDS;

            for (; i < stopSample + slopeLength; i++)
            {
                targetBufferReal[j] = sourceBufferReal[i][loBand];
                targetBufferImag[j] = sourceBufferImag[i][loBand];
                j += SBR_NUM_BANDS;
            }
        }
    }
}

#endif

#endif
