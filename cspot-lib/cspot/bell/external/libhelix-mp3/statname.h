

#ifndef _STATNAME_H
#define _STATNAME_H

#define STAT_PREFIX		xmp3

#define STATCC1(x,y,z)	STATCC2(x,y,z)
#define STATCC2(x,y,z)	x##y##z

#ifdef STAT_PREFIX
#define STATNAME(func)	STATCC1(STAT_PREFIX, _, func)
#else
#define STATNAME(func)	func
#endif

#define	CheckPadBit			STATNAME(CheckPadBit)
#define	UnpackFrameHeader	STATNAME(UnpackFrameHeader)
#define	UnpackSideInfo		STATNAME(UnpackSideInfo)
#define	AllocateBuffers		STATNAME(AllocateBuffers)
#define	FreeBuffers			STATNAME(FreeBuffers)
#define	DecodeHuffman		STATNAME(DecodeHuffman)
#define	Dequantize			STATNAME(Dequantize)
#define	IMDCT				STATNAME(IMDCT)
#define	UnpackScaleFactors	STATNAME(UnpackScaleFactors)
#define	Subband				STATNAME(Subband)

#define	samplerateTab		STATNAME(samplerateTab)
#define	bitrateTab			STATNAME(bitrateTab)
#define	samplesPerFrameTab	STATNAME(samplesPerFrameTab)
#define	bitsPerSlotTab		STATNAME(bitsPerSlotTab)
#define	sideBytesTab		STATNAME(sideBytesTab)
#define	slotTab				STATNAME(slotTab)
#define	sfBandTable			STATNAME(sfBandTable)

#endif
