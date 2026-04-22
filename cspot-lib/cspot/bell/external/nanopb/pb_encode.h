

#ifndef PB_ENCODE_H_INCLUDED
#define PB_ENCODE_H_INCLUDED

#include "pb.h"

#ifdef __cplusplus
extern "C" {
#endif

struct pb_ostream_s
{
#ifdef PB_BUFFER_ONLY

    const int *callback;
#else
    bool (*callback)(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);
#endif
    void *state;
    size_t max_size;
    size_t bytes_written;

#ifndef PB_NO_ERRMSG
    const char *errmsg;
#endif
};

bool pb_encode(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct);

#define PB_ENCODE_DELIMITED       0x02U
#define PB_ENCODE_NULLTERMINATED  0x04U
bool pb_encode_ex(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct, unsigned int flags);

#define pb_encode_delimited(s,f,d) pb_encode_ex(s,f,d, PB_ENCODE_DELIMITED)
#define pb_encode_nullterminated(s,f,d) pb_encode_ex(s,f,d, PB_ENCODE_NULLTERMINATED)

bool pb_get_encoded_size(size_t *size, const pb_msgdesc_t *fields, const void *src_struct);

pb_ostream_t pb_ostream_from_buffer(pb_byte_t *buf, size_t bufsize);

#ifndef PB_NO_ERRMSG
#define PB_OSTREAM_SIZING {0,0,0,0,0}
#else
#define PB_OSTREAM_SIZING {0,0,0,0}
#endif

bool pb_write(pb_ostream_t *stream, const pb_byte_t *buf, size_t count);

bool pb_encode_tag_for_field(pb_ostream_t *stream, const pb_field_iter_t *field);

bool pb_encode_tag(pb_ostream_t *stream, pb_wire_type_t wiretype, uint32_t field_number);

#ifndef PB_WITHOUT_64BIT
bool pb_encode_varint(pb_ostream_t *stream, uint64_t value);
#else
bool pb_encode_varint(pb_ostream_t *stream, uint32_t value);
#endif

#ifndef PB_WITHOUT_64BIT
bool pb_encode_svarint(pb_ostream_t *stream, int64_t value);
#else
bool pb_encode_svarint(pb_ostream_t *stream, int32_t value);
#endif

bool pb_encode_string(pb_ostream_t *stream, const pb_byte_t *buffer, size_t size);

bool pb_encode_fixed32(pb_ostream_t *stream, const void *value);

#ifndef PB_WITHOUT_64BIT

bool pb_encode_fixed64(pb_ostream_t *stream, const void *value);
#endif

#ifdef PB_CONVERT_DOUBLE_FLOAT

bool pb_encode_float_as_double(pb_ostream_t *stream, float value);
#endif

bool pb_encode_submessage(pb_ostream_t *stream, const pb_msgdesc_t *fields, const void *src_struct);

#ifdef __cplusplus
}
#endif

#endif
