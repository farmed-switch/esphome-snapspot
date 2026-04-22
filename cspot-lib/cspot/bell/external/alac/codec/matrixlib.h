

#ifndef __MATRIXLIB_H
#define __MATRIXLIB_H

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void	mix16( int16_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples, int32_t mixbits, int32_t mixres );
void	unmix16( int32_t * u, int32_t * v, int16_t * out, uint32_t stride, int32_t numSamples, int32_t mixbits, int32_t mixres );

void	mix20( uint8_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples, int32_t mixbits, int32_t mixres );
void	unmix20( int32_t * u, int32_t * v, uint8_t * out, uint32_t stride, int32_t numSamples, int32_t mixbits, int32_t mixres );

void	mix24( uint8_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples,
				int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted );
void	unmix24( int32_t * u, int32_t * v, uint8_t * out, uint32_t stride, int32_t numSamples,
				 int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted );

void	mix32( int32_t * in, uint32_t stride, int32_t * u, int32_t * v, int32_t numSamples,
				int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted );
void	unmix32( int32_t * u, int32_t * v, int32_t * out, uint32_t stride, int32_t numSamples,
				 int32_t mixbits, int32_t mixres, uint16_t * shiftUV, int32_t bytesShifted );

void	copy20ToPredictor( uint8_t * in, uint32_t stride, int32_t * out, int32_t numSamples );
void	copy24ToPredictor( uint8_t * in, uint32_t stride, int32_t * out, int32_t numSamples );

void	copyPredictorTo24( int32_t * in, uint8_t * out, uint32_t stride, int32_t numSamples );
void	copyPredictorTo24Shift( int32_t * in, uint16_t * shift, uint8_t * out, uint32_t stride, int32_t numSamples, int32_t bytesShifted );
void	copyPredictorTo20( int32_t * in, uint8_t * out, uint32_t stride, int32_t numSamples );

void	copyPredictorTo32( int32_t * in, int32_t * out, uint32_t stride, int32_t numSamples );
void	copyPredictorTo32Shift( int32_t * in, uint16_t * shift, int32_t * out, uint32_t stride, int32_t numSamples, int32_t bytesShifted );

#ifdef __cplusplus
}
#endif

#endif
