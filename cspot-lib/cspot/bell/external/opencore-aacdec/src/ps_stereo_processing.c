

#include    "config.h"

#ifdef AAC_PLUS

#ifdef PARAMETRICSTEREO
#include    "pv_audio_type_defs.h"
#include    "ps_stereo_processing.h"
#include    "fxp_mul32.h"
#include    "ps_all_pass_filter_coeff.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

void ps_stereo_processing(STRUCT_PS_DEC  *pms,
                          Int32          *qmfLeftReal,
                          Int32          *qmfLeftImag,
                          Int32          *qmfRightReal,
                          Int32          *qmfRightImag)
{
    Int32     group;
    Int32     subband;
    Int32     maxSubband;
    Int32     usb;
    Char     index;

    Int32  *hybrLeftReal;
    Int32  *hybrLeftImag;
    Int32  *hybrRightReal;
    Int32  *hybrRightImag;
    Int32  *ptr_hybrLeftReal;
    Int32  *ptr_hybrLeftImag;
    Int32  *ptr_hybrRightReal;
    Int32  *ptr_hybrRightImag;

    Int16   h11;
    Int16   h12;
    Int16   h21;
    Int16   h22;

    Int32   temp1;
    Int32   temp2;
    Int32   temp3;

    usb = pms->usb;

    hybrLeftReal  = pms->mHybridRealLeft;
    hybrLeftImag  = pms->mHybridImagLeft;
    hybrRightReal = pms->mHybridRealRight;
    hybrRightImag = pms->mHybridImagRight;

    for (group = 0; group < SUBQMF_GROUPS; group++)
    {

        temp1 = pms->deltaH11[group];
        temp2 = pms->deltaH12[group];

        pms->H11[group]  += temp1;
        h11  = (Int16)(pms->H11[group] >> 16);
        pms->H12[group]  += temp2;
        h12  = (Int16)(pms->H12[group] >> 16);

        temp1 = pms->deltaH21[group];
        temp2 = pms->deltaH22[group];

        pms->H21[group]  += temp1;
        h21  = (Int16)(pms->H21[group] >> 16);
        pms->H22[group]  += temp2;
        h22  = (Int16)(pms->H22[group] >> 16);

        index = groupBorders[group];

        ptr_hybrLeftReal  = &hybrLeftReal[  index];
        ptr_hybrRightReal = &hybrRightReal[ index];

        temp1 = *(ptr_hybrLeftReal) << 1;
        temp2 = *(ptr_hybrRightReal) << 1;

        temp3 = fxp_mul32_by_16(temp1, h11);
        *(ptr_hybrLeftReal)  = fxp_mac32_by_16(temp2, h21, temp3) << 1;

        temp3 = fxp_mul32_by_16(temp1, h12);
        *(ptr_hybrRightReal) = fxp_mac32_by_16(temp2, h22, temp3) << 1;

        ptr_hybrLeftImag  = &hybrLeftImag[  index];
        ptr_hybrRightImag = &hybrRightImag[ index];

        temp1 = *(ptr_hybrLeftImag) << 1;
        temp2 = *(ptr_hybrRightImag) << 1;

        temp3 = fxp_mul32_by_16(temp1, h11);
        *(ptr_hybrLeftImag)  = fxp_mac32_by_16(temp2, h21, temp3) << 1;

        temp3 = fxp_mul32_by_16(temp1, h12);
        *(ptr_hybrRightImag) = fxp_mac32_by_16(temp2, h22, temp3) << 1;

    }

    temp1 = pms->deltaH11[SUBQMF_GROUPS];
    temp2 = pms->deltaH12[SUBQMF_GROUPS];

    pms->H11[SUBQMF_GROUPS]  += temp1;
    h11  = (Int16)(pms->H11[SUBQMF_GROUPS] >> 16);
    pms->H12[SUBQMF_GROUPS]  += temp2;
    h12  = (Int16)(pms->H12[SUBQMF_GROUPS] >> 16);

    temp1 = pms->deltaH21[SUBQMF_GROUPS];
    temp2 = pms->deltaH22[SUBQMF_GROUPS];

    pms->H21[SUBQMF_GROUPS]  += temp1;
    h21  = (Int16)(pms->H21[SUBQMF_GROUPS] >> 16);
    pms->H22[SUBQMF_GROUPS]  += temp2;
    h22  = (Int16)(pms->H22[SUBQMF_GROUPS] >> 16);

    ptr_hybrLeftReal  = &qmfLeftReal[  3];
    ptr_hybrRightReal = &qmfRightReal[ 3];

    temp1 = *(ptr_hybrLeftReal) << 1;
    temp2 = *(ptr_hybrRightReal) << 1;

    temp3 = fxp_mul32_by_16(temp1, h11);
    *(ptr_hybrLeftReal)  = fxp_mac32_by_16(temp2, h21, temp3) << 1;

    temp3 = fxp_mul32_by_16(temp1, h12);
    *(ptr_hybrRightReal)  = fxp_mac32_by_16(temp2, h22, temp3) << 1;

    ptr_hybrLeftImag  = &qmfLeftImag[  3];
    ptr_hybrRightImag = &qmfRightImag[ 3];

    temp1 = *(ptr_hybrLeftImag) << 1;
    temp2 = *(ptr_hybrRightImag) << 1;

    temp3 = fxp_mul32_by_16(temp1, h11);
    *(ptr_hybrLeftImag)  = fxp_mac32_by_16(temp2, h21, temp3) << 1;

    temp3 = fxp_mul32_by_16(temp1, h12);
    *(ptr_hybrRightImag)  = fxp_mac32_by_16(temp2, h22, temp3) << 1;

    for (group = SUBQMF_GROUPS + 1; group < NO_IID_GROUPS; group++)
    {
        temp1 = pms->deltaH11[group];
        temp2 = pms->deltaH12[group];

        pms->H11[group]  += temp1;
        h11  = (Int16)(pms->H11[group] >> 16);
        pms->H12[group]  += temp2;
        h12  = (Int16)(pms->H12[group] >> 16);

        temp1 = pms->deltaH21[group];
        temp2 = pms->deltaH22[group];

        pms->H21[group]  += temp1;
        h21  = (Int16)(pms->H21[group] >> 16);
        pms->H22[group]  += temp2;
        h22  = (Int16)(pms->H22[group] >> 16);

        index = groupBorders[group];
        maxSubband = groupBorders[group + 1];
        maxSubband = min(usb, maxSubband);

        ptr_hybrLeftReal  = &qmfLeftReal[  index];
        ptr_hybrRightReal = &qmfRightReal[ index];

        for (subband = index; subband < maxSubband; subband++)
        {
            temp1 = *(ptr_hybrLeftReal) << 1;
            temp2 = *(ptr_hybrRightReal) << 1;
            temp3 = fxp_mul32_by_16(temp1, h11);
            *(ptr_hybrLeftReal++)   = fxp_mac32_by_16(temp2, h21, temp3) << 1;

            temp3 = fxp_mul32_by_16(temp1, h12);
            *(ptr_hybrRightReal++)  = fxp_mac32_by_16(temp2, h22, temp3) << 1;
        }

        ptr_hybrLeftImag  = &qmfLeftImag[  index];
        ptr_hybrRightImag = &qmfRightImag[ index];

        for (subband = index; subband < maxSubband; subband++)
        {
            temp1 = *(ptr_hybrLeftImag) << 1;
            temp2 = *(ptr_hybrRightImag) << 1;
            temp3 = fxp_mul32_by_16(temp1, h11);
            *(ptr_hybrLeftImag++)   = fxp_mac32_by_16(temp2, h21, temp3) << 1;

            temp3 = fxp_mul32_by_16(temp1, h12);
            *(ptr_hybrRightImag++)  = fxp_mac32_by_16(temp2, h22, temp3) << 1;

        }

    }

}

#endif

#endif

