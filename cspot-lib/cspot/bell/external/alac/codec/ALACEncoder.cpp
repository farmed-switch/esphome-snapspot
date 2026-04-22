

#define VERBOSE_DEBUG		0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ALACEncoder.h"

#include "aglib.h"
#include "dplib.h"
#include "matrixlib.h"

#include "ALACBitUtilities.h"
#include "ALACAudioTypes.h"
#include "EndianPortable.h"

#if (__GNUC__) > 4 || defined (__APPLE__)
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif
#if !defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

typedef int16_t (*SearchCoefs)[kALACMaxCoefs];

const uint32_t kALACEncoderMagic	= 'dpge';
const uint32_t kMaxSampleSize		= 32;
const uint32_t kDefaultMixBits	= 2;
const uint32_t kDefaultMixRes		= 0;
const uint32_t kMaxRes			= 4;
const uint32_t kDefaultNumUV		= 8;
const uint32_t kMinUV				= 4;
const uint32_t kMaxUV				= 8;

#if VERBOSE_DEBUG
static void AddFiller( BitBuffer * bits, int32_t numBytes );
#endif

static const uint32_t	sChannelMaps[kALACMaxChannels] =
{
	ID_SCE,
	ID_CPE,
	(ID_CPE << 3) | (ID_SCE),
	(ID_SCE << 9) | (ID_CPE << 3) | (ID_SCE),
	(ID_CPE << 9) | (ID_CPE << 3) | (ID_SCE),
	(ID_SCE << 15) | (ID_CPE << 9) | (ID_CPE << 3) | (ID_SCE),
	(ID_SCE << 18) | (ID_SCE << 15) | (ID_CPE << 9) | (ID_CPE << 3) | (ID_SCE),
	(ID_SCE << 21) | (ID_CPE << 15) | (ID_CPE << 9) | (ID_CPE << 3) | (ID_SCE)
};

static const uint32_t sSupportediPodSampleRates[] =
{
	8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
};

ALACEncoder::ALACEncoder() :
	mBitDepth( 0 ),
    mFastMode( 0 ),
	mMixBufferU( nil ),
	mMixBufferV( nil ),
	mPredictorU( nil ),
	mPredictorV( nil ),
	mShiftBufferUV( nil ),
	mWorkBuffer( nil ),

	mTotalBytesGenerated( 0 ),
	mAvgBitRate( 0 ),
	mMaxFrameBytes( 0 )
{

	mFrameSize = kALACDefaultFrameSize;
}

ALACEncoder::~ALACEncoder()
{

	if ( mMixBufferU )
    {
		free(mMixBufferU);
        mMixBufferU = NULL;
    }
	if ( mMixBufferV )
    {
		free(mMixBufferV);
        mMixBufferV = NULL;
    }

	if ( mPredictorU )
    {
		free(mPredictorU);
        mPredictorU = NULL;
    }
	if ( mPredictorV )
    {
		free(mPredictorV);
        mPredictorV = NULL;
    }

	if ( mShiftBufferUV )
    {
		free(mShiftBufferUV);
        mShiftBufferUV = NULL;
    }

	if ( mWorkBuffer )
    {
		free(mWorkBuffer);
        mWorkBuffer = NULL;
    }
}

#if PRAGMA_MARK
#pragma mark -
#endif

int32_t ALACEncoder::EncodeStereo( BitBuffer * bitstream, void * inputBuffer, uint32_t stride, uint32_t channelIndex, uint32_t numSamples )
{
	BitBuffer		workBits;
	BitBuffer		startBits = *bitstream;
	AGParamRec		agParams;
	uint32_t          bits1, bits2;
	uint32_t			dilate;
	int32_t			mixBits, mixRes, maxRes;
	uint32_t			minBits, minBits1, minBits2;
	uint32_t			numU, numV;
	uint32_t			mode;
	uint32_t			pbFactor;
	uint32_t			chanBits;
	uint32_t			denShift;
	uint8_t			bytesShifted;
	SearchCoefs		coefsU;
	SearchCoefs		coefsV;
	uint32_t			index;
	uint8_t			partialFrame;
	uint32_t			escapeBits;
	bool			doEscape;
	int32_t		status = ALAC_noErr;

	RequireAction( (mBitDepth == 16) || (mBitDepth == 20) || (mBitDepth == 24) || (mBitDepth == 32), return kALAC_ParamError; );

	coefsU = (SearchCoefs) mCoefsU[channelIndex];
	coefsV = (SearchCoefs) mCoefsV[channelIndex];

	if ( mBitDepth == 32 )
		bytesShifted = 2;
	else if ( mBitDepth >= 24 )
		bytesShifted = 1;
	else
		bytesShifted = 0;

	chanBits = mBitDepth - (bytesShifted * 8) + 1;

	partialFrame = (numSamples == mFrameSize) ? 0 : 1;

	mixBits		= kDefaultMixBits;
	maxRes		= kMaxRes;
	numU = numV = kDefaultNumUV;
	denShift	= DENSHIFT_DEFAULT;
	mode		= 0;
	pbFactor	= 4;
	dilate		= 8;

	minBits	= minBits1 = minBits2 = 1ul << 31;

    int32_t		bestRes = mLastMixRes[channelIndex];

    for ( mixRes = 0; mixRes <= maxRes; mixRes++ )
    {

        switch ( mBitDepth )
        {
            case 16:
                mix16( (int16_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples/dilate, mixBits, mixRes );
                break;
            case 20:
                mix20( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples/dilate, mixBits, mixRes );
                break;
            case 24:

                mix24( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples/dilate,
                        mixBits, mixRes, mShiftBufferUV, bytesShifted );
                break;
            case 32:

                mix32( (int32_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples/dilate,
                        mixBits, mixRes, mShiftBufferUV, bytesShifted );
                break;
        }

        BitBufferInit( &workBits, mWorkBuffer, mMaxOutputBytes );

        pc_block( mMixBufferU, mPredictorU, numSamples/dilate, coefsU[numU - 1], numU, chanBits, DENSHIFT_DEFAULT );
        pc_block( mMixBufferV, mPredictorV, numSamples/dilate, coefsV[numV - 1], numV, chanBits, DENSHIFT_DEFAULT );

        set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples/dilate, numSamples/dilate, MAX_RUN_DEFAULT );
        status = dyn_comp( &agParams, mPredictorU, &workBits, numSamples/dilate, chanBits, &bits1 );
        RequireNoErr( status, goto Exit; );

        set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples/dilate, numSamples/dilate, MAX_RUN_DEFAULT );
        status = dyn_comp( &agParams, mPredictorV, &workBits, numSamples/dilate, chanBits, &bits2 );
        RequireNoErr( status, goto Exit; );

        if ( (bits1 + bits2) < minBits1 )
        {
            minBits1 = bits1 + bits2;
            bestRes = mixRes;
        }
    }

    mLastMixRes[channelIndex] = (int16_t)bestRes;

	mixRes = mLastMixRes[channelIndex];
	switch ( mBitDepth )
	{
		case 16:
			mix16( (int16_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, mixBits, mixRes );
			break;
		case 20:
			mix20( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, mixBits, mixRes );
			break;
		case 24:

			mix24( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples,
					mixBits, mixRes, mShiftBufferUV, bytesShifted );
			break;
		case 32:

			mix32( (int32_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples,
					mixBits, mixRes, mShiftBufferUV, bytesShifted );
			break;
	}

	numU = numV = kMinUV;
	minBits1 = minBits2 = 1ul << 31;

	for ( uint32_t numUV = kMinUV; numUV <= kMaxUV; numUV += 4 )
	{
		BitBufferInit( &workBits, mWorkBuffer, mMaxOutputBytes );

		dilate = 32;

		for ( uint32_t converge = 0; converge < 8; converge++ )
		{
		    pc_block( mMixBufferU, mPredictorU, numSamples/dilate, coefsU[numUV-1], numUV, chanBits, DENSHIFT_DEFAULT );
		    pc_block( mMixBufferV, mPredictorV, numSamples/dilate, coefsV[numUV-1], numUV, chanBits, DENSHIFT_DEFAULT );
		}

		dilate = 8;

		set_ag_params( &agParams, MB0, (pbFactor * PB0)/4, KB0, numSamples/dilate, numSamples/dilate, MAX_RUN_DEFAULT );
		status = dyn_comp( &agParams, mPredictorU, &workBits, numSamples/dilate, chanBits, &bits1 );

		if ( (bits1 * dilate + 16 * numUV) < minBits1 )
		{
			minBits1 = bits1 * dilate + 16 * numUV;
			numU = numUV;
		}

		set_ag_params( &agParams, MB0, (pbFactor * PB0)/4, KB0, numSamples/dilate, numSamples/dilate, MAX_RUN_DEFAULT );
		status = dyn_comp( &agParams, mPredictorV, &workBits, numSamples/dilate, chanBits, &bits2 );

		if ( (bits2 * dilate + 16 * numUV) < minBits2 )
		{
			minBits2 = bits2 * dilate + 16 * numUV;
			numV = numUV;
		}
	}

	minBits = minBits1 + minBits2 + (8   * 8) + ((partialFrame == true) ? 32 : 0);
	if ( bytesShifted != 0 )
		minBits += (numSamples * (bytesShifted * 8) * 2);

	escapeBits = (numSamples * mBitDepth * 2) + ((partialFrame == true) ? 32 : 0) + (2 * 8);

	doEscape = (minBits >= escapeBits) ? true : false;

	if ( doEscape == false )
	{

		BitBufferWrite( bitstream, 0, 12 );
		BitBufferWrite( bitstream, (partialFrame << 3) | (bytesShifted << 1), 4 );
		if ( partialFrame )
			BitBufferWrite( bitstream, numSamples, 32 );
		BitBufferWrite( bitstream, mixBits, 8 );
		BitBufferWrite( bitstream, mixRes, 8 );

		BitBufferWrite( bitstream, (mode << 4) | DENSHIFT_DEFAULT, 8 );
		BitBufferWrite( bitstream, (pbFactor << 5) | numU, 8 );
		for ( index = 0; index < numU; index++ )
			BitBufferWrite( bitstream, coefsU[numU - 1][index], 16 );

		BitBufferWrite( bitstream, (mode << 4) | DENSHIFT_DEFAULT, 8 );
		BitBufferWrite( bitstream, (pbFactor << 5) | numV, 8 );
		for ( index = 0; index < numV; index++ )
			BitBufferWrite( bitstream, coefsV[numV - 1][index], 16 );

		if ( bytesShifted != 0 )
		{
			uint32_t		bitShift = bytesShifted * 8;

			for ( index = 0; index < (numSamples * 2); index += 2 )
			{
				uint32_t			shiftedVal;

				shiftedVal = ((uint32_t)mShiftBufferUV[index + 0] << bitShift) | (uint32_t)mShiftBufferUV[index + 1];
				BitBufferWrite( bitstream, shiftedVal, bitShift * 2 );
			}
		}

		if ( mode == 0 )
		{
			pc_block( mMixBufferU, mPredictorU, numSamples, coefsU[numU - 1], numU, chanBits, DENSHIFT_DEFAULT );
		}
		else
		{
			pc_block( mMixBufferU, mPredictorV, numSamples, coefsU[numU - 1], numU, chanBits, DENSHIFT_DEFAULT );
			pc_block( mPredictorV, mPredictorU, numSamples, nil, 31, chanBits, 0 );
		}

		set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples, numSamples, MAX_RUN_DEFAULT );
		status = dyn_comp( &agParams, mPredictorU, bitstream, numSamples, chanBits, &bits1 );
		RequireNoErr( status, goto Exit; );

		if ( mode == 0 )
		{
			pc_block( mMixBufferV, mPredictorV, numSamples, coefsV[numV - 1], numV, chanBits, DENSHIFT_DEFAULT );
		}
		else
		{
			pc_block( mMixBufferV, mPredictorU, numSamples, coefsV[numV - 1], numV, chanBits, DENSHIFT_DEFAULT );
			pc_block( mPredictorU, mPredictorV, numSamples, nil, 31, chanBits, 0 );
		}

		set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples, numSamples, MAX_RUN_DEFAULT );
		status = dyn_comp( &agParams, mPredictorV, bitstream, numSamples, chanBits, &bits2 );
		RequireNoErr( status, goto Exit; );

		minBits = BitBufferGetPosition( bitstream ) - BitBufferGetPosition( &startBits );
		if ( minBits >= escapeBits )
		{
			*bitstream = startBits;
			doEscape = true;
			printf( "compressed frame too big: %u vs. %u \n", minBits, escapeBits );
		}
	}

	if ( doEscape == true )
	{

		status = this->EncodeStereoEscape( bitstream, inputBuffer, stride, numSamples );

#if VERBOSE_DEBUG
		DebugMsg( "escape!: %lu vs %lu", minBits, escapeBits );
#endif
	}

Exit:
	return status;
}

int32_t ALACEncoder::EncodeStereoFast( BitBuffer * bitstream, void * inputBuffer, uint32_t stride, uint32_t channelIndex, uint32_t numSamples )
{
	BitBuffer		startBits = *bitstream;
	AGParamRec		agParams;
	uint32_t	bits1, bits2;
	int32_t			mixBits, mixRes;
	uint32_t			minBits, minBits1, minBits2;
	uint32_t			numU, numV;
	uint32_t			mode;
	uint32_t			pbFactor;
	uint32_t			chanBits;
	uint32_t			denShift;
	uint8_t			bytesShifted;
	SearchCoefs		coefsU;
	SearchCoefs		coefsV;
	uint32_t			index;
	uint8_t			partialFrame;
	uint32_t			escapeBits;
	bool			doEscape;
	int32_t		status;

	RequireAction( (mBitDepth == 16) || (mBitDepth == 20) || (mBitDepth == 24) || (mBitDepth == 32), return kALAC_ParamError; );

	coefsU = (SearchCoefs) mCoefsU[channelIndex];
	coefsV = (SearchCoefs) mCoefsV[channelIndex];

	if ( mBitDepth == 32 )
		bytesShifted = 2;
	else if ( mBitDepth >= 24 )
		bytesShifted = 1;
	else
		bytesShifted = 0;

	chanBits = mBitDepth - (bytesShifted * 8) + 1;

	partialFrame = (numSamples == mFrameSize) ? 0 : 1;

	mixBits		= kDefaultMixBits;
	mixRes		= kDefaultMixRes;
	numU = numV = kDefaultNumUV;
	denShift	= DENSHIFT_DEFAULT;
	mode		= 0;
	pbFactor	= 4;

	minBits	= minBits1 = minBits2 = 1ul << 31;

	switch ( mBitDepth )
	{
		case 16:
			mix16( (int16_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, mixBits, mixRes );
			break;
		case 20:
			mix20( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, mixBits, mixRes );
			break;
		case 24:

			mix24( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples,
					mixBits, mixRes, mShiftBufferUV, bytesShifted );
			break;
		case 32:

			mix32( (int32_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples,
					mixBits, mixRes, mShiftBufferUV, bytesShifted );
			break;
	}

	BitBufferWrite( bitstream, 0, 12 );
	BitBufferWrite( bitstream, (partialFrame << 3) | (bytesShifted << 1), 4 );
	if ( partialFrame )
		BitBufferWrite( bitstream, numSamples, 32 );
	BitBufferWrite( bitstream, mixBits, 8 );
	BitBufferWrite( bitstream, mixRes, 8 );

	BitBufferWrite( bitstream, (mode << 4) | DENSHIFT_DEFAULT, 8 );
	BitBufferWrite( bitstream, (pbFactor << 5) | numU, 8 );
	for ( index = 0; index < numU; index++ )
		BitBufferWrite( bitstream, coefsU[numU - 1][index], 16 );

	BitBufferWrite( bitstream, (mode << 4) | DENSHIFT_DEFAULT, 8 );
	BitBufferWrite( bitstream, (pbFactor << 5) | numV, 8 );
	for ( index = 0; index < numV; index++ )
		BitBufferWrite( bitstream, coefsV[numV - 1][index], 16 );

	if ( bytesShifted != 0 )
	{
		uint32_t		bitShift = bytesShifted * 8;

		for ( index = 0; index < (numSamples * 2); index += 2 )
		{
			uint32_t			shiftedVal;

			shiftedVal = ((uint32_t)mShiftBufferUV[index + 0] << bitShift) | (uint32_t)mShiftBufferUV[index + 1];
			BitBufferWrite( bitstream, shiftedVal, bitShift * 2 );
		}
	}

	pc_block( mMixBufferU, mPredictorU, numSamples, coefsU[numU - 1], numU, chanBits, DENSHIFT_DEFAULT );

	set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples, numSamples, MAX_RUN_DEFAULT );
	status = dyn_comp( &agParams, mPredictorU, bitstream, numSamples, chanBits, &bits1 );
	RequireNoErr( status, goto Exit; );

	pc_block( mMixBufferV, mPredictorV, numSamples, coefsV[numV - 1], numV, chanBits, DENSHIFT_DEFAULT );

	set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples, numSamples, MAX_RUN_DEFAULT );
	status = dyn_comp( &agParams, mPredictorV, bitstream, numSamples, chanBits, &bits2 );
	RequireNoErr( status, goto Exit; );

	minBits1 = bits1 + (numU * sizeof(int16_t) * 8);
	minBits2 = bits2 + (numV * sizeof(int16_t) * 8);

	minBits = minBits1 + minBits2 + (8   * 8) + ((partialFrame == true) ? 32 : 0);
	if ( bytesShifted != 0 )
		minBits += (numSamples * (bytesShifted * 8) * 2);

	escapeBits = (numSamples * mBitDepth * 2) + ((partialFrame == true) ? 32 : 0) + (2 * 8);

	doEscape = (minBits >= escapeBits) ? true : false;

	if ( doEscape == false )
	{

		minBits = BitBufferGetPosition( bitstream ) - BitBufferGetPosition( &startBits );
		if ( minBits >= escapeBits )
		{
			doEscape = true;
			printf( "compressed frame too big: %u vs. %u\n", minBits, escapeBits );
		}

	}

	if ( doEscape == true )
	{

		*bitstream = startBits;

		status = this->EncodeStereoEscape( bitstream, inputBuffer, stride, numSamples );

#if VERBOSE_DEBUG
		DebugMsg( "escape!: %u vs %u", minBits, (numSamples * mBitDepth * 2) );
#endif
	}

Exit:
	return status;
}

int32_t ALACEncoder::EncodeStereoEscape( BitBuffer * bitstream, void * inputBuffer, uint32_t stride, uint32_t numSamples )
{
	int16_t *		input16;
	int32_t *		input32;
	uint8_t			partialFrame;
	uint32_t			index;

	partialFrame = (numSamples == mFrameSize) ? 0 : 1;

	BitBufferWrite( bitstream, 0, 12 );
	BitBufferWrite( bitstream, (partialFrame << 3) | 1, 4 );
	if ( partialFrame )
		BitBufferWrite( bitstream, numSamples, 32 );

	switch ( mBitDepth )
	{
		case 16:
			input16 = (int16_t *) inputBuffer;

			for ( index = 0; index < (numSamples * stride); index += stride )
			{
				BitBufferWrite( bitstream, input16[index + 0], 16 );
				BitBufferWrite( bitstream, input16[index + 1], 16 );
			}
			break;
		case 20:

			mix20( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, 0, 0 );
			for ( index = 0; index < numSamples; index++ )
			{
				BitBufferWrite( bitstream, mMixBufferU[index], 20 );
				BitBufferWrite( bitstream, mMixBufferV[index], 20 );
			}
			break;
		case 24:

			mix24( (uint8_t *) inputBuffer, stride, mMixBufferU, mMixBufferV, numSamples, 0, 0, mShiftBufferUV, 0 );
			for ( index = 0; index < numSamples; index++ )
			{
				BitBufferWrite( bitstream, mMixBufferU[index], 24 );
				BitBufferWrite( bitstream, mMixBufferV[index], 24 );
			}
			break;
		case 32:
			input32 = (int32_t *) inputBuffer;

			for ( index = 0; index < (numSamples * stride); index += stride )
			{
				BitBufferWrite( bitstream, input32[index + 0], 32 );
				BitBufferWrite( bitstream, input32[index + 1], 32 );
			}
			break;
	}

	return ALAC_noErr;
}

int32_t ALACEncoder::EncodeMono( BitBuffer * bitstream, void * inputBuffer, uint32_t stride, uint32_t channelIndex, uint32_t numSamples )
{
	BitBuffer		startBits = *bitstream;
	AGParamRec		agParams;
	uint32_t	bits1;
	uint32_t			numU;
	SearchCoefs		coefsU;
	uint32_t			dilate;
	uint32_t			minBits, bestU;
	uint32_t			minU, maxU;
	uint32_t			index, index2;
	uint8_t			bytesShifted;
	uint32_t			shift;
	uint32_t			mask;
	uint32_t			chanBits;
	uint8_t			pbFactor;
	uint8_t			partialFrame;
	int16_t *		input16;
	int32_t *		input32;
	uint32_t			escapeBits;
	bool			doEscape;
	int32_t		status;

	RequireAction( (mBitDepth == 16) || (mBitDepth == 20) || (mBitDepth == 24) || (mBitDepth == 32), return kALAC_ParamError; );

	status = ALAC_noErr;

	coefsU = (SearchCoefs) mCoefsU[channelIndex];

	if ( mBitDepth == 32 )
		bytesShifted = 2;
	else if ( mBitDepth >= 24 )
		bytesShifted = 1;
	else
		bytesShifted = 0;

	shift = bytesShifted * 8;
	mask = (1ul << shift) - 1;
	chanBits = mBitDepth - (bytesShifted * 8);

	partialFrame = (numSamples == mFrameSize) ? 0 : 1;

	switch ( mBitDepth )
	{
		case 16:
		{

			input16 = (int16_t *) inputBuffer;
			for ( index = 0, index2 = 0; index < numSamples; index++, index2 += stride )
				mMixBufferU[index] = (int32_t) input16[index2];
			break;
		}
		case 20:

			copy20ToPredictor( (uint8_t *) inputBuffer, stride, mMixBufferU, numSamples );
			break;
		case 24:

			copy24ToPredictor( (uint8_t *) inputBuffer, stride, mMixBufferU, numSamples );
			for ( index = 0; index < numSamples; index++ )
			{
				mShiftBufferUV[index] = (uint16_t)(mMixBufferU[index] & mask);
				mMixBufferU[index] >>= shift;
			}
			break;
		case 32:
		{

			input32 = (int32_t *) inputBuffer;

			for ( index = 0, index2 = 0; index < numSamples; index++, index2 += stride )
			{
				int32_t			val = input32[index2];

				mShiftBufferUV[index] = (uint16_t)(val & mask);
				mMixBufferU[index] = val >> shift;
			}
			break;
		}
	}

	minU		= 4;
	maxU		= 8;
	minBits		= 1ul << 31;
	pbFactor	= 4;

	minBits	= 1ul << 31;
	bestU	= minU;

	for ( numU = minU; numU <= maxU; numU += 4 )
	{
		BitBuffer		workBits;
		uint32_t			numBits;

		BitBufferInit( &workBits, mWorkBuffer, mMaxOutputBytes );

		dilate = 32;
		for ( uint32_t converge = 0; converge < 7; converge++ )
			pc_block( mMixBufferU, mPredictorU, numSamples/dilate, coefsU[numU-1], numU, chanBits, DENSHIFT_DEFAULT );

		dilate = 8;
		pc_block( mMixBufferU, mPredictorU, numSamples/dilate, coefsU[numU-1], numU, chanBits, DENSHIFT_DEFAULT );

		set_ag_params( &agParams, MB0, (pbFactor * PB0) / 4, KB0, numSamples/dilate, numSamples/dilate, MAX_RUN_DEFAULT );
		status = dyn_comp( &agParams, mPredictorU, &workBits, numSamples/dilate, chanBits, &bits1 );
		RequireNoErr( status, goto Exit; );

		numBits = (dilate * bits1) + (16 * numU);
		if ( numBits < minBits )
		{
			bestU	= numU;
			minBits = numBits;
		}
	}

	minBits += (4   * 8) + ((partialFrame == true) ? 32 : 0);
	if ( bytesShifted != 0 )
		minBits += (numSamples * (bytesShifted * 8));

	escapeBits = (numSamples * mBitDepth) + ((partialFrame == true) ? 32 : 0) + (2 * 8);

	doEscape = (minBits >= escapeBits) ? true : false;

	if ( doEscape == false )
	{

		BitBufferWrite( bitstream, 0, 12 );
		BitBufferWrite( bitstream, (partialFrame << 3) | (bytesShifted << 1), 4 );
		if ( partialFrame )
			BitBufferWrite( bitstream, numSamples, 32 );
		BitBufferWrite( bitstream, 0, 16 );

		numU = bestU;
		BitBufferWrite( bitstream, (0 << 4) | DENSHIFT_DEFAULT, 8 );
		BitBufferWrite( bitstream, (pbFactor << 5) | numU, 8 );
		for ( index = 0; index < numU; index++ )
			BitBufferWrite( bitstream, coefsU[numU-1][index], 16 );

		if ( bytesShifted != 0 )
		{
			for ( index = 0; index < numSamples; index++ )
				BitBufferWrite( bitstream, mShiftBufferUV[index], shift );
		}

		pc_block( mMixBufferU, mPredictorU, numSamples, coefsU[numU-1], numU, chanBits, DENSHIFT_DEFAULT );

		set_standard_ag_params( &agParams, numSamples, numSamples );
		status = dyn_comp( &agParams, mPredictorU, bitstream, numSamples, chanBits, &bits1 );

		minBits = BitBufferGetPosition( bitstream ) - BitBufferGetPosition( &startBits );
		if ( minBits >= escapeBits )
		{
			*bitstream = startBits;
			doEscape = true;
			printf( "compressed frame too big: %u vs. %u\n", minBits, escapeBits );
		}
	}

	if ( doEscape == true )
	{

		BitBufferWrite( bitstream, 0, 12 );
		BitBufferWrite( bitstream, (partialFrame << 3) | 1, 4 );
		if ( partialFrame )
			BitBufferWrite( bitstream, numSamples, 32 );

		switch ( mBitDepth )
		{
			case 16:
				input16 = (int16_t *) inputBuffer;
				for ( index = 0; index < (numSamples * stride); index += stride )
					BitBufferWrite( bitstream, input16[index], 16 );
				break;
			case 20:

				copy20ToPredictor( (uint8_t *) inputBuffer, stride, mMixBufferU, numSamples );
				for ( index = 0; index < numSamples; index++ )
					BitBufferWrite( bitstream, mMixBufferU[index], 20 );
				break;
			case 24:

				copy24ToPredictor( (uint8_t *) inputBuffer, stride, mMixBufferU, numSamples );
				for ( index = 0; index < numSamples; index++ )
					BitBufferWrite( bitstream, mMixBufferU[index], 24 );
				break;
			case 32:
				input32 = (int32_t *) inputBuffer;
				for ( index = 0; index < (numSamples * stride); index += stride )
					BitBufferWrite( bitstream, input32[index], 32 );
				break;
		}
#if VERBOSE_DEBUG
		DebugMsg( "escape!: %lu vs %lu", minBits, (numSamples * mBitDepth) );
#endif
	}

Exit:
	return status;
}

#if PRAGMA_MARK
#pragma mark -
#endif

int32_t ALACEncoder::Encode(AudioFormatDescription theInputFormat, AudioFormatDescription theOutputFormat,
                             unsigned char * theReadBuffer, unsigned char * theWriteBuffer, int32_t * ioNumBytes)
{
	uint32_t				numFrames;
	uint32_t				outputSize;
	BitBuffer			bitstream;
	int32_t			status;

	numFrames = *ioNumBytes/theInputFormat.mBytesPerPacket;

	BitBufferInit( &bitstream, theWriteBuffer, mMaxOutputBytes );

	if ( theInputFormat.mChannelsPerFrame == 2 )
	{

		BitBufferWrite( &bitstream, ID_CPE, 3 );
		BitBufferWrite( &bitstream, 0, 4 );

		if ( mFastMode == false )
			status = this->EncodeStereo( &bitstream, theReadBuffer, 2, 0, numFrames );
		else
			status = this->EncodeStereoFast( &bitstream, theReadBuffer, 2, 0, numFrames );
		RequireNoErr( status, goto Exit; );
	}
	else if ( theInputFormat.mChannelsPerFrame == 1 )
	{

		BitBufferWrite( &bitstream, ID_SCE, 3 );
		BitBufferWrite( &bitstream, 0, 4 );

		status = this->EncodeMono( &bitstream, theReadBuffer, 1, 0, numFrames );
		RequireNoErr( status, goto Exit; );
	}
	else
	{
		char *					inputBuffer;
		uint32_t				tag;
		uint32_t				channelIndex;
		uint32_t				inputIncrement;
		uint8_t				stereoElementTag;
		uint8_t				monoElementTag;
		uint8_t				lfeElementTag;

		inputBuffer		= (char *) theReadBuffer;
		inputIncrement	= ((mBitDepth + 7) / 8);

		stereoElementTag	= 0;
		monoElementTag		= 0;
		lfeElementTag		= 0;

		for ( channelIndex = 0; channelIndex < theInputFormat.mChannelsPerFrame; )
		{
			tag = (sChannelMaps[theInputFormat.mChannelsPerFrame - 1] & (0x7ul << (channelIndex * 3))) >> (channelIndex * 3);

			BitBufferWrite( &bitstream, tag, 3 );
			switch ( tag )
			{
				case ID_SCE:

					BitBufferWrite( &bitstream, monoElementTag, 4 );

					status = this->EncodeMono( &bitstream, inputBuffer, theInputFormat.mChannelsPerFrame, channelIndex, numFrames );

					inputBuffer += inputIncrement;
					channelIndex++;
					monoElementTag++;
					break;

				case ID_CPE:

					BitBufferWrite( &bitstream, stereoElementTag, 4 );

					status = this->EncodeStereo( &bitstream, inputBuffer, theInputFormat.mChannelsPerFrame, channelIndex, numFrames );

					inputBuffer += (inputIncrement * 2);
					channelIndex += 2;
					stereoElementTag++;
					break;

				case ID_LFE:

					BitBufferWrite( &bitstream, lfeElementTag, 4 );

					status = this->EncodeMono( &bitstream, inputBuffer, theInputFormat.mChannelsPerFrame, channelIndex, numFrames );

					inputBuffer += inputIncrement;
					channelIndex++;
					lfeElementTag++;
					break;

				default:
					printf( "That ain't right! (%u)\n", tag );
					status = kALAC_ParamError;
					goto Exit;
			}

			RequireNoErr( status, goto Exit; );
		}
	}

#if VERBOSE_DEBUG
{

	int32_t			bitsLeft;
	int32_t			bytesLeft;

	bitsLeft = BitBufferGetPosition( &bitstream ) - 3;
	bytesLeft = bitstream.byteSize - ((bitsLeft + 7) / 8);

	if ( (bytesLeft > 20) && ((bytesLeft & 0x4u) != 0) )
		AddFiller( &bitstream, bytesLeft );
}
#endif

	BitBufferWrite( &bitstream, ID_END, 3 );

	BitBufferByteAlign( &bitstream, true );

	outputSize = BitBufferGetPosition( &bitstream ) / 8;

	*ioNumBytes = outputSize;

	mTotalBytesGenerated += outputSize;
	mMaxFrameBytes = MAX( mMaxFrameBytes, outputSize );

	status = ALAC_noErr;

Exit:
	return status;
}

int32_t ALACEncoder::Finish()
{

	return ALAC_noErr;
}

#if PRAGMA_MARK
#pragma mark -
#endif

void ALACEncoder::GetConfig( ALACSpecificConfig & config )
{
	config.frameLength			= Swap32NtoB(mFrameSize);
	config.compatibleVersion	= (uint8_t) kALACCompatibleVersion;
	config.bitDepth				= (uint8_t) mBitDepth;
	config.pb					= (uint8_t) PB0;
	config.kb					= (uint8_t) KB0;
	config.mb					= (uint8_t) MB0;
	config.numChannels			= (uint8_t) mNumChannels;
	config.maxRun				= Swap16NtoB((uint16_t) MAX_RUN_DEFAULT);
	config.maxFrameBytes		= Swap32NtoB(mMaxFrameBytes);
	config.avgBitRate			= Swap32NtoB(mAvgBitRate);
	config.sampleRate			= Swap32NtoB(mOutputSampleRate);
}

uint32_t ALACEncoder::GetMagicCookieSize(uint32_t inNumChannels)
{
    if (inNumChannels > 2)
    {
        return sizeof(ALACSpecificConfig) + kChannelAtomSize + sizeof(ALACAudioChannelLayout);
    }
    else
    {
        return sizeof(ALACSpecificConfig);
    }
}

void ALACEncoder::GetMagicCookie(void * outCookie, uint32_t * ioSize)
{
    ALACSpecificConfig theConfig = {0};
    ALACAudioChannelLayout theChannelLayout = {0};
    uint8_t theChannelAtom[kChannelAtomSize] = {0, 0, 0, 0, 'c', 'h', 'a', 'n', 0, 0, 0, 0};
    uint32_t theCookieSize = sizeof(ALACSpecificConfig);
    uint8_t * theCookiePointer = (uint8_t *)outCookie;

    GetConfig(theConfig);
    if (theConfig.numChannels > 2)
    {
        theChannelLayout.mChannelLayoutTag = ALACChannelLayoutTags[theConfig.numChannels - 1];
        theCookieSize += (sizeof(ALACAudioChannelLayout) + kChannelAtomSize);
    }
     if (*ioSize >= theCookieSize)
    {
        memcpy(theCookiePointer, &theConfig, sizeof(ALACSpecificConfig));
        theChannelAtom[3] = (sizeof(ALACAudioChannelLayout) + kChannelAtomSize);
        if (theConfig.numChannels > 2)
        {
            theCookiePointer += sizeof(ALACSpecificConfig);
            memcpy(theCookiePointer, theChannelAtom, kChannelAtomSize);
            theCookiePointer += kChannelAtomSize;
            memcpy(theCookiePointer, &theChannelLayout, sizeof(ALACAudioChannelLayout));
        }
        *ioSize = theCookieSize;
    }
    else
    {
        *ioSize = 0;
    }
}

int32_t ALACEncoder::InitializeEncoder(AudioFormatDescription theOutputFormat)
{
	int32_t			status;

    mOutputSampleRate = theOutputFormat.mSampleRate;
    mNumChannels = theOutputFormat.mChannelsPerFrame;
    switch(theOutputFormat.mFormatFlags)
    {
        case 1:
            mBitDepth = 16;
            break;
        case 2:
            mBitDepth = 20;
            break;
        case 3:
            mBitDepth = 24;
            break;
        case 4:
            mBitDepth = 32;
            break;
        default:
            break;
    }

	for ( uint32_t index = 0; index < kALACMaxChannels; index++ )
		mLastMixRes[index] = kDefaultMixRes;

	mMaxOutputBytes = mFrameSize * mNumChannels * ((10 + kMaxSampleSize) / 8)  + 1;

	mMixBufferU = (int32_t *) calloc( mFrameSize * sizeof(int32_t), 1 );
	mMixBufferV = (int32_t *) calloc( mFrameSize * sizeof(int32_t), 1 );

	mPredictorU = (int32_t *) calloc( mFrameSize * sizeof(int32_t), 1 );
	mPredictorV = (int32_t *) calloc( mFrameSize * sizeof(int32_t), 1 );

	mShiftBufferUV = (uint16_t *) calloc( mFrameSize * 2 * sizeof(uint16_t),1 );

	mWorkBuffer = (uint8_t *) calloc( mMaxOutputBytes, 1 );

	RequireAction( (mMixBufferU != nil) && (mMixBufferV != nil) &&
					(mPredictorU != nil) && (mPredictorV != nil) &&
					(mShiftBufferUV != nil) && (mWorkBuffer != nil ),
					status = kALAC_MemFullError; goto Exit; );

	status = ALAC_noErr;

	for ( int32_t channel = 0; channel < (int32_t)mNumChannels; channel++ )
	{
		for ( int32_t search = 0; search < kALACMaxSearches; search++ )
		{
			init_coefs( mCoefsU[channel][search], DENSHIFT_DEFAULT, kALACMaxCoefs );
			init_coefs( mCoefsV[channel][search], DENSHIFT_DEFAULT, kALACMaxCoefs );
		}
	}

Exit:
	return status;
}

void ALACEncoder::GetSourceFormat( const AudioFormatDescription * source, AudioFormatDescription * output )
{

	if ( (source->mFormatID != kALACFormatLinearPCM) || ((source->mFormatFlags & kALACFormatFlagIsFloat) != 0) ||
		( source->mBitsPerChannel <= 16 ) )
		mBitDepth = 16;
	else if ( source->mBitsPerChannel <= 20 )
		mBitDepth = 20;
	else if ( source->mBitsPerChannel <= 24 )
		mBitDepth = 24;
	else
		mBitDepth = 32;

}

#if VERBOSE_DEBUG

#if PRAGMA_MARK
#pragma mark -
#endif

static void AddFiller( BitBuffer * bits, int32_t numBytes )
{
	uint8_t		tag;
	uint32_t		index;

	numBytes -= 6;
	if ( numBytes <= 0 )
		return;

	tag = (numBytes & 0x8) ? ID_FIL : ID_DSE;

	BitBufferWrite( bits, tag, 3 );
	if ( tag == ID_FIL )
	{

		numBytes = (numBytes > 269) ? 269 : numBytes;

		if ( numBytes >= 15 )
		{
			uint16_t			extensionSize;

			BitBufferWrite( bits, 15, 4 );

			extensionSize = (numBytes - 15) + 1;
			Assert( extensionSize <= 255 );
			BitBufferWrite( bits, extensionSize, 8 );
		}
		else
			BitBufferWrite( bits, numBytes, 4 );

		BitBufferWrite( bits, 0x10, 8 );
		for ( index = 0; index < (numBytes - 1); index++ )
			BitBufferWrite( bits, 0xa5, 8 );
	}
	else
	{

		numBytes = (numBytes > 510) ? 510 : numBytes;

		BitBufferWrite( bits, 0, 4 );
		BitBufferWrite( bits, 1, 1 );

		if ( numBytes >= 255 )
		{
			BitBufferWrite( bits, 255, 8 );
			BitBufferWrite( bits, numBytes - 255, 8 );
		}
		else
			BitBufferWrite( bits, numBytes, 8 );

		BitBufferByteAlign( bits, true );

		for ( index = 0; index < numBytes; index++ )
			BitBufferWrite( bits, 0x5a, 8 );
	}
}

#endif
