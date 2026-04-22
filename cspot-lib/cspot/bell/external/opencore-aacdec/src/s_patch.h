

#ifndef S_PATCH_H
#define S_PATCH_H

#define MAX_NUM_PATCHES   6

#define     SBR_NUM_COLUMNS      38
#define     SBR_NUM_BANDS        48
#define     SBR_NUM_BANDS_OVR_4 (SBR_NUM_BANDS>>2)

struct PATCH
{
    Int32 noOfPatches;
    Int32 targetStartBand[MAX_NUM_PATCHES];
};

#endif

