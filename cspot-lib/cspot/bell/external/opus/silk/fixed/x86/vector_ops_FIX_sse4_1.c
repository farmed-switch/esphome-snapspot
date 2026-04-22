

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include "main.h"

#include "SigProc_FIX.h"
#include "pitch.h"

opus_int64 silk_inner_prod16_aligned_64_sse4_1(
    const opus_int16            *inVec1,
    const opus_int16            *inVec2,
    const opus_int              len
)
{
    opus_int  i, dataSize8;
    opus_int64 sum;

    __m128i xmm_tempa;
    __m128i inVec1_76543210, acc1;
    __m128i inVec2_76543210, acc2;

    sum = 0;
    dataSize8 = len & ~7;

    acc1 = _mm_setzero_si128();
    acc2 = _mm_setzero_si128();

    for( i = 0; i < dataSize8; i += 8 ) {
        inVec1_76543210 = _mm_loadu_si128( (__m128i *)(&inVec1[i + 0] ) );
        inVec2_76543210 = _mm_loadu_si128( (__m128i *)(&inVec2[i + 0] ) );

        inVec1_76543210 = _mm_madd_epi16( inVec1_76543210, inVec2_76543210 );

        xmm_tempa       = _mm_cvtepi32_epi64( inVec1_76543210 );

        inVec1_76543210 = _mm_shuffle_epi32( inVec1_76543210, _MM_SHUFFLE( 0, 0, 3, 2 ) );
        inVec1_76543210 = _mm_cvtepi32_epi64( inVec1_76543210 );

        acc1 = _mm_add_epi64( acc1, xmm_tempa );
        acc2 = _mm_add_epi64( acc2, inVec1_76543210 );
    }

    acc1 = _mm_add_epi64( acc1, acc2 );

    acc2 = _mm_shuffle_epi32( acc1, _MM_SHUFFLE( 0, 0, 3, 2 ) );
    acc1 = _mm_add_epi64( acc1, acc2 );

    _mm_storel_epi64( (__m128i *)&sum, acc1 );

    for( ; i < len; i++ ) {
        sum = silk_SMLABB( sum, inVec1[ i ], inVec2[ i ] );
    }

    return sum;
}
