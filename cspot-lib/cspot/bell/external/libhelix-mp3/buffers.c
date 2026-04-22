

#include <stdlib.h>
#include <string.h>
#include "coder.h"

#define ClearBuffer(buf, nBytes) memset(buf, 0, nBytes)

MP3DecInfo *AllocateBuffers(void)
{
	MP3DecInfo *mp3DecInfo;
	FrameHeader *fh;
	SideInfo *si;
	ScaleFactorInfo *sfi;
	HuffmanInfo *hi;
	DequantInfo *di;
	IMDCTInfo *mi;
	SubbandInfo *sbi;

	mp3DecInfo = (MP3DecInfo *)malloc(sizeof(MP3DecInfo));
	if (!mp3DecInfo)
		return 0;
	ClearBuffer(mp3DecInfo, sizeof(MP3DecInfo));

	fh =  (FrameHeader *)     malloc(sizeof(FrameHeader));
	si =  (SideInfo *)        malloc(sizeof(SideInfo));
	sfi = (ScaleFactorInfo *) malloc(sizeof(ScaleFactorInfo));
	hi =  (HuffmanInfo *)     malloc(sizeof(HuffmanInfo));
	di =  (DequantInfo *)     malloc(sizeof(DequantInfo));
	mi =  (IMDCTInfo *)       malloc(sizeof(IMDCTInfo));
	sbi = (SubbandInfo *)     malloc(sizeof(SubbandInfo));

	mp3DecInfo->FrameHeaderPS =     (void *)fh;
	mp3DecInfo->SideInfoPS =        (void *)si;
	mp3DecInfo->ScaleFactorInfoPS = (void *)sfi;
	mp3DecInfo->HuffmanInfoPS =     (void *)hi;
	mp3DecInfo->DequantInfoPS =     (void *)di;
	mp3DecInfo->IMDCTInfoPS =       (void *)mi;
	mp3DecInfo->SubbandInfoPS =     (void *)sbi;

	if (!fh || !si || !sfi || !hi || !di || !mi || !sbi) {
		FreeBuffers(mp3DecInfo);
		return 0;
	}

	ClearBuffer(fh,  sizeof(FrameHeader));
	ClearBuffer(si,  sizeof(SideInfo));
	ClearBuffer(sfi, sizeof(ScaleFactorInfo));
	ClearBuffer(hi,  sizeof(HuffmanInfo));
	ClearBuffer(di,  sizeof(DequantInfo));
	ClearBuffer(mi,  sizeof(IMDCTInfo));
	ClearBuffer(sbi, sizeof(SubbandInfo));

	return mp3DecInfo;
}

#define SAFE_FREE(x)	{if (x)	free(x);	(x) = 0;}

void FreeBuffers(MP3DecInfo *mp3DecInfo)
{
	if (!mp3DecInfo)
		return;

	SAFE_FREE(mp3DecInfo->FrameHeaderPS);
	SAFE_FREE(mp3DecInfo->SideInfoPS);
	SAFE_FREE(mp3DecInfo->ScaleFactorInfoPS);
	SAFE_FREE(mp3DecInfo->HuffmanInfoPS);
	SAFE_FREE(mp3DecInfo->DequantInfoPS);
	SAFE_FREE(mp3DecInfo->IMDCTInfoPS);
	SAFE_FREE(mp3DecInfo->SubbandInfoPS);

	SAFE_FREE(mp3DecInfo);
}
