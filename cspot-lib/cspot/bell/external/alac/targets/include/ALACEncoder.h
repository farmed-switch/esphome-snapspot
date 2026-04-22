

#pragma once

#include <stdint.h>

#include "ALACAudioTypes.h"

struct BitBuffer;

class ALACEncoder
{
	public:
		ALACEncoder();
		virtual ~ALACEncoder();

		virtual int32_t	Encode(AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat,
                                   unsigned char * theReadBuffer, unsigned char * theWriteBuffer, int32_t * ioNumBytes);
		virtual int32_t	Finish( );

		void				SetFastMode( bool fast ) { mFastMode = fast; };

		void				SetFrameSize( uint32_t frameSize ) { mFrameSize = frameSize; };

		void				GetConfig( ALACSpecificConfig & config );
        uint32_t            GetMagicCookieSize(uint32_t inNumChannels);
        void				GetMagicCookie( void * config, uint32_t * ioSize );

        virtual int32_t	InitializeEncoder(AudioFormatDescription theOutputFormat);

    protected:
		virtual void		GetSourceFormat( const AudioFormatDescription * source, AudioFormatDescription * output );

		int32_t			EncodeStereo( struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );
		int32_t			EncodeStereoFast( struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );
		int32_t			EncodeStereoEscape( struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t numSamples );
		int32_t			EncodeMono( struct BitBuffer * bitstream, void * input, uint32_t stride, uint32_t channelIndex, uint32_t numSamples );

		int16_t					mBitDepth;
		bool					mFastMode;

		int16_t					mLastMixRes[kALACMaxChannels];

		int32_t *				mMixBufferU;
		int32_t *				mMixBufferV;
		int32_t *				mPredictorU;
		int32_t *				mPredictorV;
		uint16_t *				mShiftBufferUV;

		uint8_t *					mWorkBuffer;

		int16_t					mCoefsU[kALACMaxChannels][kALACMaxSearches][kALACMaxCoefs];
		int16_t					mCoefsV[kALACMaxChannels][kALACMaxSearches][kALACMaxCoefs];

		uint32_t					mTotalBytesGenerated;
		uint32_t					mAvgBitRate;
		uint32_t					mMaxFrameBytes;
        uint32_t                  mFrameSize;
        uint32_t                  mMaxOutputBytes;
        uint32_t                  mNumChannels;
        uint32_t                  mOutputSampleRate;
};
