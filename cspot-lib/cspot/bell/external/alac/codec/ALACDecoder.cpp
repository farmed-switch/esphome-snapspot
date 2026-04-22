

#include <stdlib.h>
#include <string.h>

#include "ALACDecoder.h"

#include "dplib.h"
#include "aglib.h"
#include "matrixlib.h"

#include "ALACBitUtilities.h"
#include "EndianPortable.h"

#if (__GNUC__) > 4 || defined (__APPLE__)
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif
#if !defined(__APPLE__)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

const uint32_t kMaxBitDepth = 32;

static void Zero16( int16_t * buffer, uint32_t numItems, uint32_t stride );
static void Zero24( uint8_t * buffer, uint32_t numItems, uint32_t stride );
static void Zero32( int32_t * buffer, uint32_t numItems, uint32_t stride );

ALACDecoder::ALACDecoder() :
	mMixBufferU( nil ),
	mMixBufferV( nil ),
	mPredictor( nil ),
	mShiftBuffer( nil )
{
	memset( &mConfig, 0, sizeof(mConfig) );
}

ALACDecoder::~ALACDecoder()
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

	if ( mPredictor )
    {
		free(mPredictor);
        mPredictor = NULL;
    }
}

int32_t ALACDecoder::Init( void * inMagicCookie, uint32_t inMagicCookieSize )
{
	int32_t		status = ALAC_noErr;
    ALACSpecificConfig theConfig;
    uint8_t * theActualCookie = (uint8_t *)inMagicCookie;
    uint32_t theCookieBytesRemaining = inMagicCookieSize;

    if (theActualCookie[4] == 'f' && theActualCookie[5] == 'r' && theActualCookie[6] == 'm' && theActualCookie[7] == 'a')
    {
        theActualCookie += 12;
        theCookieBytesRemaining -= 12;
    }

    if (theActualCookie[4] == 'a' && theActualCookie[5] == 'l' && theActualCookie[6] == 'a' && theActualCookie[7] == 'c')
    {
        theActualCookie += 12;
        theCookieBytesRemaining -= 12;
    }

    if (theCookieBytesRemaining >= sizeof(ALACSpecificConfig))
    {
        theConfig.frameLength = Swap32BtoN(((ALACSpecificConfig *)theActualCookie)->frameLength);
        theConfig.compatibleVersion = ((ALACSpecificConfig *)theActualCookie)->compatibleVersion;
        theConfig.bitDepth = ((ALACSpecificConfig *)theActualCookie)->bitDepth;
        theConfig.pb = ((ALACSpecificConfig *)theActualCookie)->pb;
        theConfig.mb = ((ALACSpecificConfig *)theActualCookie)->mb;
        theConfig.kb = ((ALACSpecificConfig *)theActualCookie)->kb;
        theConfig.numChannels = ((ALACSpecificConfig *)theActualCookie)->numChannels;
        theConfig.maxRun = Swap16BtoN(((ALACSpecificConfig *)theActualCookie)->maxRun);
        theConfig.maxFrameBytes = Swap32BtoN(((ALACSpecificConfig *)theActualCookie)->maxFrameBytes);
        theConfig.avgBitRate = Swap32BtoN(((ALACSpecificConfig *)theActualCookie)->avgBitRate);
        theConfig.sampleRate = Swap32BtoN(((ALACSpecificConfig *)theActualCookie)->sampleRate);

        mConfig = theConfig;

        RequireAction( mConfig.compatibleVersion <= kALACVersion, return kALAC_ParamError; );

        mMixBufferU = (int32_t *) calloc( mConfig.frameLength * sizeof(int32_t), 1 );
        mMixBufferV = (int32_t *) calloc( mConfig.frameLength * sizeof(int32_t), 1 );

        mPredictor = (int32_t *) calloc( mConfig.frameLength * sizeof(int32_t), 1 );

        mShiftBuffer = (uint16_t *) mPredictor;

        RequireAction( (mMixBufferU != nil) && (mMixBufferV != nil) && (mPredictor != nil),
                        status = kALAC_MemFullError; goto Exit; );
     }
    else
    {
        status = kALAC_ParamError;
    }

Exit:
	return status;
}

int32_t ALACDecoder::Decode( BitBuffer * bits, uint8_t * sampleBuffer, uint32_t numSamples, uint32_t numChannels, uint32_t * outNumSamples )
{
	BitBuffer			shiftBits;
	uint32_t            bits1, bits2;
	uint8_t				tag;
	uint8_t				elementInstanceTag;
	AGParamRec			agParams;
	uint32_t				channelIndex;
	int16_t				coefsU[32];
	int16_t				coefsV[32];
	uint8_t				numU, numV;
	uint8_t				mixBits;
	int8_t				mixRes;
	uint16_t			unusedHeader;
	uint8_t				escapeFlag;
	uint32_t			chanBits;
	uint8_t				bytesShifted;
	uint32_t			shift;
	uint8_t				modeU, modeV;
	uint32_t			denShiftU, denShiftV;
	uint16_t			pbFactorU, pbFactorV;
	uint16_t			pb;
	int16_t *			samples;
	int16_t *			out16;
	uint8_t *			out20;
	uint8_t *			out24;
	int32_t *			out32;
	uint8_t				headerByte;
	uint8_t				partialFrame;
	uint32_t			extraBits;
	int32_t				val;
	uint32_t			i, j;
	int32_t             status;

	RequireAction( (bits != nil) && (sampleBuffer != nil) && (outNumSamples != nil), return kALAC_ParamError; );
	RequireAction( numChannels > 0, return kALAC_ParamError; );

	mActiveElements = 0;
	channelIndex	= 0;

	samples = (int16_t *) sampleBuffer;

	status = ALAC_noErr;
	*outNumSamples = numSamples;

	while ( status == ALAC_noErr )
	{

    	RequireAction( bits->cur < bits->end, status = kALAC_ParamError; goto Exit; );

		pb = mConfig.pb;

		tag = BitBufferReadSmall( bits, 3 );
		switch ( tag )
		{
			case ID_SCE:
			case ID_LFE:
			{

				elementInstanceTag = BitBufferReadSmall( bits, 4 );
				mActiveElements |= (1u << elementInstanceTag);

				unusedHeader = (uint16_t) BitBufferRead( bits, 12 );
				RequireAction( unusedHeader == 0, status = kALAC_ParamError; goto Exit; );

				headerByte = (uint8_t) BitBufferRead( bits, 4 );

				partialFrame = headerByte >> 3;

				bytesShifted = (headerByte >> 1) & 0x3u;
				RequireAction( bytesShifted != 3, status = kALAC_ParamError; goto Exit; );

				shift = bytesShifted * 8;

				escapeFlag = headerByte & 0x1;

				chanBits = mConfig.bitDepth - (bytesShifted * 8);

				if ( partialFrame != 0 )
				{
					numSamples  = BitBufferRead( bits, 16 ) << 16;
					numSamples |= BitBufferRead( bits, 16 );
				}

				if ( escapeFlag == 0 )
				{

					mixBits	= (uint8_t) BitBufferRead( bits, 8 );
					mixRes	= (int8_t) BitBufferRead( bits, 8 );

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					modeU		= headerByte >> 4;
					denShiftU	= headerByte & 0xfu;

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					pbFactorU	= headerByte >> 5;
					numU		= headerByte & 0x1fu;

					for ( i = 0; i < numU; i++ )
						coefsU[i] = (int16_t) BitBufferRead( bits, 16 );

					if ( bytesShifted != 0 )
					{
						shiftBits = *bits;
						BitBufferAdvance( bits, (bytesShifted * 8) * numSamples );
					}

					set_ag_params( &agParams, mConfig.mb, (pb * pbFactorU) / 4, mConfig.kb, numSamples, numSamples, mConfig.maxRun );
					status = dyn_decomp( &agParams, bits, mPredictor, numSamples, chanBits, &bits1 );
					RequireNoErr( status, goto Exit; );

					if ( modeU == 0 )
					{
						unpc_block( mPredictor, mMixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
					else
					{

						unpc_block( mPredictor, mPredictor, numSamples, nil, 31, chanBits, 0 );
						unpc_block( mPredictor, mMixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
				}
				else
				{

					shift = 32 - chanBits;
					if ( chanBits <= 16 )
					{
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( bits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mMixBufferU[i] = val;
						}
					}
					else
					{

						extraBits = chanBits - 16;
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( bits, 16 );
							val = (val << 16) >> shift;
							mMixBufferU[i] = val | BitBufferRead( bits, (uint8_t) extraBits );
						}
					}

					mixBits = mixRes = 0;
					bits1 = chanBits * numSamples;
					bytesShifted = 0;
				}

				if ( bytesShifted != 0 )
				{
					shift = bytesShifted * 8;

					for ( i = 0; i < numSamples; i++ )
						mShiftBuffer[i] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
				}

				switch ( mConfig.bitDepth )
				{
					case 16:
						out16 = &((int16_t *)sampleBuffer)[channelIndex];
						for ( i = 0, j = 0; i < numSamples; i++, j += numChannels )
							out16[j] = (int16_t) mMixBufferU[i];
						break;
					case 20:
						out20 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						copyPredictorTo20( mMixBufferU, out20, numChannels, numSamples );
						break;
					case 24:
						out24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						if ( bytesShifted != 0 )
							copyPredictorTo24Shift( mMixBufferU, mShiftBuffer, out24, numChannels, numSamples, bytesShifted );
						else
							copyPredictorTo24( mMixBufferU, out24, numChannels, numSamples );
						break;
					case 32:
						out32 = &((int32_t *)sampleBuffer)[channelIndex];
						if ( bytesShifted != 0 )
							copyPredictorTo32Shift( mMixBufferU, mShiftBuffer, out32, numChannels, numSamples, bytesShifted );
						else
							copyPredictorTo32( mMixBufferU, out32, numChannels, numSamples);
						break;
				}

				channelIndex += 1;
				*outNumSamples = numSamples;
				break;
			}

			case ID_CPE:
			{

				if ( (channelIndex + 2) > numChannels )
					goto NoMoreChannels;

				elementInstanceTag = BitBufferReadSmall( bits, 4 );
				mActiveElements |= (1u << elementInstanceTag);

				unusedHeader = (uint16_t) BitBufferRead( bits, 12 );
				RequireAction( unusedHeader == 0, status = kALAC_ParamError; goto Exit; );

				headerByte = (uint8_t) BitBufferRead( bits, 4 );

				partialFrame = headerByte >> 3;

				bytesShifted = (headerByte >> 1) & 0x3u;
				RequireAction( bytesShifted != 3, status = kALAC_ParamError; goto Exit; );

				shift = bytesShifted * 8;

				escapeFlag = headerByte & 0x1;

				chanBits = mConfig.bitDepth - (bytesShifted * 8) + 1;

				if ( partialFrame != 0 )
				{
					numSamples  = BitBufferRead( bits, 16 ) << 16;
					numSamples |= BitBufferRead( bits, 16 );
				}

				if ( escapeFlag == 0 )
				{

					mixBits		= (uint8_t) BitBufferRead( bits, 8 );
					mixRes		= (int8_t) BitBufferRead( bits, 8 );

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					modeU		= headerByte >> 4;
					denShiftU	= headerByte & 0xfu;

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					pbFactorU	= headerByte >> 5;
					numU		= headerByte & 0x1fu;
					for ( i = 0; i < numU; i++ )
						coefsU[i] = (int16_t) BitBufferRead( bits, 16 );

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					modeV		= headerByte >> 4;
					denShiftV	= headerByte & 0xfu;

					headerByte	= (uint8_t) BitBufferRead( bits, 8 );
					pbFactorV	= headerByte >> 5;
					numV		= headerByte & 0x1fu;
					for ( i = 0; i < numV; i++ )
						coefsV[i] = (int16_t) BitBufferRead( bits, 16 );

					if ( bytesShifted != 0 )
					{
						shiftBits = *bits;
						BitBufferAdvance( bits, (bytesShifted * 8) * 2 * numSamples );
					}

					set_ag_params( &agParams, mConfig.mb, (pb * pbFactorU) / 4, mConfig.kb, numSamples, numSamples, mConfig.maxRun );
					status = dyn_decomp( &agParams, bits, mPredictor, numSamples, chanBits, &bits1 );
					RequireNoErr( status, goto Exit; );

					if ( modeU == 0 )
					{
						unpc_block( mPredictor, mMixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}
					else
					{

						unpc_block( mPredictor, mPredictor, numSamples, nil, 31, chanBits, 0 );
						unpc_block( mPredictor, mMixBufferU, numSamples, &coefsU[0], numU, chanBits, denShiftU );
					}

					set_ag_params( &agParams, mConfig.mb, (pb * pbFactorV) / 4, mConfig.kb, numSamples, numSamples, mConfig.maxRun );
					status = dyn_decomp( &agParams, bits, mPredictor, numSamples, chanBits, &bits2 );
					RequireNoErr( status, goto Exit; );

					if ( modeV == 0 )
					{
						unpc_block( mPredictor, mMixBufferV, numSamples, &coefsV[0], numV, chanBits, denShiftV );
					}
					else
					{

						unpc_block( mPredictor, mPredictor, numSamples, nil, 31, chanBits, 0 );
						unpc_block( mPredictor, mMixBufferV, numSamples, &coefsV[0], numV, chanBits, denShiftV );
					}
				}
				else
				{

					chanBits = mConfig.bitDepth;
					shift = 32 - chanBits;
					if ( chanBits <= 16 )
					{
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( bits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mMixBufferU[i] = val;

							val = (int32_t) BitBufferRead( bits, (uint8_t) chanBits );
							val = (val << shift) >> shift;
							mMixBufferV[i] = val;
						}
					}
					else
					{

						extraBits = chanBits - 16;
						for ( i = 0; i < numSamples; i++ )
						{
							val = (int32_t) BitBufferRead( bits, 16 );
							val = (val << 16) >> shift;
							mMixBufferU[i] = val | BitBufferRead( bits, (uint8_t)extraBits );

							val = (int32_t) BitBufferRead( bits, 16 );
							val = (val << 16) >> shift;
							mMixBufferV[i] = val | BitBufferRead( bits, (uint8_t)extraBits );
						}
					}

					bits1 = chanBits * numSamples;
					bits2 = chanBits * numSamples;
					mixBits = mixRes = 0;
					bytesShifted = 0;
				}

				if ( bytesShifted != 0 )
				{
					shift = bytesShifted * 8;

					for ( i = 0; i < (numSamples * 2); i += 2 )
					{
						mShiftBuffer[i + 0] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
						mShiftBuffer[i + 1] = (uint16_t) BitBufferRead( &shiftBits, (uint8_t) shift );
					}
				}

				switch ( mConfig.bitDepth )
				{
					case 16:
						out16 = &((int16_t *)sampleBuffer)[channelIndex];
						unmix16( mMixBufferU, mMixBufferV, out16, numChannels, numSamples, mixBits, mixRes );
						break;
					case 20:
						out20 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						unmix20( mMixBufferU, mMixBufferV, out20, numChannels, numSamples, mixBits, mixRes );
						break;
					case 24:
						out24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
						unmix24( mMixBufferU, mMixBufferV, out24, numChannels, numSamples,
									mixBits, mixRes, mShiftBuffer, bytesShifted );
						break;
					case 32:
						out32 = &((int32_t *)sampleBuffer)[channelIndex];
						unmix32( mMixBufferU, mMixBufferV, out32, numChannels, numSamples,
									mixBits, mixRes, mShiftBuffer, bytesShifted );
						break;
				}

				channelIndex += 2;
				*outNumSamples = numSamples;
				break;
			}

			case ID_CCE:
			case ID_PCE:
			{

				status = kALAC_ParamError;
				break;
			}

			case ID_DSE:
			{

				status = this->DataStreamElement( bits );
				break;
			}

			case ID_FIL:
			{

				status = this->FillElement( bits );
				break;
			}

			case ID_END:
			{

				BitBufferByteAlign( bits, false );

				goto Exit;
			}
		}

#if ! DEBUG

		if ( channelIndex >= numChannels )
			break;
#endif
	}

NoMoreChannels:

	for ( ; channelIndex < numChannels; channelIndex++ )
	{
		switch ( mConfig.bitDepth )
		{
			case 16:
			{
				int16_t *	fill16 = &((int16_t *)sampleBuffer)[channelIndex];
				Zero16( fill16, numSamples, numChannels );
				break;
			}
			case 24:
			{
				uint8_t *	fill24 = (uint8_t *)sampleBuffer + (channelIndex * 3);
				Zero24( fill24, numSamples, numChannels );
				break;
			}
			case 32:
			{
				int32_t *	fill32 = &((int32_t *)sampleBuffer)[channelIndex];
				Zero32( fill32, numSamples, numChannels );
				break;
			}
		}
	}

Exit:
	return status;
}

#if PRAGMA_MARK
#pragma mark -
#endif

int32_t ALACDecoder::FillElement( BitBuffer * bits )
{
	int16_t		count;

	count = BitBufferReadSmall( bits, 4 );
	if ( count == 15 )
		count += (int16_t) BitBufferReadSmall( bits, 8 ) - 1;

	BitBufferAdvance( bits, count * 8 );

	RequireAction( bits->cur <= bits->end, return kALAC_ParamError; );

	return ALAC_noErr;
}

int32_t ALACDecoder::DataStreamElement( BitBuffer * bits )
{
	uint8_t		element_instance_tag;
	int32_t		data_byte_align_flag;
	uint16_t		count;

	element_instance_tag = BitBufferReadSmall( bits, 4 );

	data_byte_align_flag = BitBufferReadOne( bits );

	count = BitBufferReadSmall( bits, 8 );
	if ( count == 255 )
		count += BitBufferReadSmall( bits, 8 );

	if ( data_byte_align_flag )
		BitBufferByteAlign( bits, false );

	BitBufferAdvance( bits, count * 8 );

	RequireAction( bits->cur <= bits->end, return kALAC_ParamError; );

	return ALAC_noErr;
}

static void Zero16( int16_t * buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * sizeof(int16_t) );
	}
	else
	{
		for ( uint32_t index = 0; index < (numItems * stride); index += stride )
			buffer[index] = 0;
	}
}

static void Zero24( uint8_t * buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * 3 );
	}
	else
	{
		for ( uint32_t index = 0; index < (numItems * stride * 3); index += (stride * 3) )
		{
			buffer[index + 0] = 0;
			buffer[index + 1] = 0;
			buffer[index + 2] = 0;
		}
	}
}

static void Zero32( int32_t * buffer, uint32_t numItems, uint32_t stride )
{
	if ( stride == 1 )
	{
		memset( buffer, 0, numItems * sizeof(int32_t) );
	}
	else
	{
		for ( uint32_t index = 0; index < (numItems * stride); index += stride )
			buffer[index] = 0;
	}
}
