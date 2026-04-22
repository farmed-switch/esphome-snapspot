

#include "opus_types.h"
#include "opus_defines.h"

#if !defined(_entcode_H)
# define _entcode_H (1)
# include <limits.h>
# include <stddef.h>
# include "ecintrin.h"

extern const opus_uint32 SMALL_DIV_TABLE[129];

#ifdef OPUS_ARM_ASM
#define USE_SMALL_DIV_TABLE
#endif

typedef opus_uint32           ec_window;
typedef struct ec_ctx         ec_ctx;
typedef struct ec_ctx         ec_enc;
typedef struct ec_ctx         ec_dec;

# define EC_WINDOW_SIZE ((int)sizeof(ec_window)*CHAR_BIT)

# define EC_UINT_BITS   (8)

# define BITRES 3

struct ec_ctx{

   unsigned char *buf;

   opus_uint32    storage;

   opus_uint32    end_offs;

   ec_window      end_window;

   int            nend_bits;

   int            nbits_total;

   opus_uint32    offs;

   opus_uint32    rng;

   opus_uint32    val;

   opus_uint32    ext;

   int            rem;

   int            error;
};

static OPUS_INLINE opus_uint32 ec_range_bytes(ec_ctx *_this){
  return _this->offs;
}

static OPUS_INLINE unsigned char *ec_get_buffer(ec_ctx *_this){
  return _this->buf;
}

static OPUS_INLINE int ec_get_error(ec_ctx *_this){
  return _this->error;
}

static OPUS_INLINE int ec_tell(ec_ctx *_this){
  return _this->nbits_total-EC_ILOG(_this->rng);
}

opus_uint32 ec_tell_frac(ec_ctx *_this);

static OPUS_INLINE opus_uint32 celt_udiv(opus_uint32 n, opus_uint32 d) {
   celt_sig_assert(d>0);
#ifdef USE_SMALL_DIV_TABLE
   if (d>256)
      return n/d;
   else {
      opus_uint32 t, q;
      t = EC_ILOG(d&-d);
      q = (opus_uint64)SMALL_DIV_TABLE[d>>t]*(n>>(t-1))>>32;
      return q+(n-q*d >= d);
   }
#else
   return n/d;
#endif
}

static OPUS_INLINE opus_int32 celt_sudiv(opus_int32 n, opus_int32 d) {
   celt_sig_assert(d>0);
#ifdef USE_SMALL_DIV_TABLE
   if (n<0)
      return -(opus_int32)celt_udiv(-n, d);
   else
      return celt_udiv(n, d);
#else
   return n/d;
#endif
}

#endif
