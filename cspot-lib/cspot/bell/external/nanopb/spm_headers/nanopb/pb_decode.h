

#ifndef PB_DECODE_H_INCLUDED
#define PB_DECODE_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pb_istream_s
{
#ifdef PB_BUFFER_ONLY

    int *callback;
#else
    bool (*callback)(pb_istream_t *stream, pb_byte_t *buf, size_t count);
#endif

    void *state;
    size_t bytes_left;

#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

#ifndef PB_NO_ERRMSG
#define PB_ISTREAM_EMPTY {0,0,0,0}
#else
#define PB_ISTREAM_EMPTY {0,0,0}
#endif

bool pb_decode(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct);

#define PB_DECODE_NOINIT          0x01U
#define PB_DECODE_DELIMITED       0x02U
#define PB_DECODE_NULLTERMINATED  0x04U
bool pb_decode_ex(pb_istream_t *stream, const pb_msgdesc_t *fields, void *dest_struct, unsigned int flags);

#define pb_decode_noinit(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_NOINIT)
#define pb_decode_delimited(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_DELIMITED)
#define pb_decode_delimited_noinit(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_DELIMITED | PB_DECODE_NOINIT)
#define pb_decode_nullterminated(s,f,d) pb_decode_ex(s,f,d, PB_DECODE_NULLTERMINATED)

#ifdef PB_ENABLE_MALLOC

void pb_release(const pb_msgdesc_t *fields, void *dest_struct);
#else

#define pb_release(fields, dest_struct) PB_UNUSED(fields); PB_UNUSED(dest_struct);
#endif

pb_istream_t pb_istream_from_buffer(const pb_byte_t *buf, size_t msglen);

bool pb_read(pb_istream_t *stream, pb_byte_t *buf, size_t count);

bool pb_decode_tag(pb_istream_t *stream, pb_wire_type_t *wire_type, uint32_t *tag, bool *eof);

bool pb_skip_field(pb_istream_t *stream, pb_wire_type_t wire_type);

#ifndef PB_WITHOUT_64BIT
bool pb_decode_varint(pb_istream_t *stream, uint64_t *dest);
#else
#define pb_decode_varint pb_decode_varint32
#endif

bool pb_decode_varint32(pb_istream_t *stream, uint32_t *dest);

bool pb_decode_bool(pb_istream_t *stream, bool *dest);

#ifndef PB_WITHOUT_64BIT
bool pb_decode_svarint(pb_istream_t *stream, int64_t *dest);
#else
bool pb_decode_svarint(pb_istream_t *stream, int32_t *dest);
#endif

bool pb_decode_fixed32(pb_istream_t *stream, void *dest);

#ifndef PB_WITHOUT_64BIT

bool pb_decode_fixed64(pb_istream_t *stream, void *dest);
#endif

#ifdef PB_CONVERT_DOUBLE_FLOAT

bool pb_decode_double_as_float(pb_istream_t *stream, float *dest);
#endif

bool pb_make_string_substream(pb_istream_t *stream, pb_istream_t *substream);
bool pb_close_string_substream(pb_istream_t *stream, pb_istream_t *substream);

#ifdef __cplusplus
}
#endif

#endif
