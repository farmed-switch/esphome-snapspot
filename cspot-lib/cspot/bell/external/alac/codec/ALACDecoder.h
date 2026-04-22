

#ifndef _ALACDECODER_H
#define _ALACDECODER_H

#if PRAGMA_ONCE
#pragma once
#endif

#include <stdint.h>

#include "ALACAudioTypes.h"

struct BitBuffer;

class ALACDecoder
{
	public:
		ALACDecoder();
		~ALACDecoder();

		int32_t	Init( void * inMagicCookie, uint32_t inMagicCookieSize );
		int32_t	Decode( struct BitBuffer * bits, uint8_t * sampleBuffer, uint32_t numSamples, uint32_t numChannels, uint32_t * outNumSamples );

	public:

		ALACSpecificConfig		mConfig;

	protected:
		int32_t	FillElement( struct BitBuffer * bits );
		int32_t	DataStreamElement( struct BitBuffer * bits );

		uint16_t					mActiveElements;

		int32_t *				mMixBufferU;
		int32_t *				mMixBufferV;
		int32_t *				mPredictor;
		uint16_t *				mShiftBuffer;

};

#endif
