

#include <pb_decode.h>
#include <pb_encode.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <malloc_wrappers.h>
#include "random_data.h"
#include "validation.h"
#include "flakystream.h"
#include "test_helpers.h"
#include "alltypes_static.pb.h"
#include "alltypes_pointer.pb.h"
#include "alltypes_callback.pb.h"
#include "alltypes_proto3_static.pb.h"
#include "alltypes_proto3_pointer.pb.h"

#ifndef FUZZTEST_BUFSIZE
#define FUZZTEST_BUFSIZE 256*1024
#endif
#ifndef FUZZTEST_MAX_STANDALONE_BUFSIZE
#define FUZZTEST_MAX_STANDALONE_BUFSIZE 16384
#endif
static size_t g_bufsize = FUZZTEST_BUFSIZE;

#if !defined(FUZZTEST_PROTO2_STATIC) && \
    !defined(FUZZTEST_PROTO3_STATIC) && \
    !defined(FUZZTEST_PROTO2_POINTER) && \
    !defined(FUZZTEST_PROTO3_POINTER) && \
    !defined(FUZZTEST_IO_ERRORS)
#define FUZZTEST_PROTO2_STATIC
#define FUZZTEST_PROTO3_STATIC
#define FUZZTEST_PROTO2_POINTER
#define FUZZTEST_PROTO3_POINTER
#define FUZZTEST_IO_ERRORS
#endif

static uint32_t xor32_checksum(const void *data, size_t len)
{
    const uint8_t *buf = (const uint8_t*)data;
    uint32_t checksum = 1234;
    for (; len > 0; len--)
    {
        checksum ^= checksum << 13;
        checksum ^= checksum >> 17;
        checksum ^= checksum << 5;
        checksum += *buf++;
    }
    return checksum;
}

static bool do_decode(const uint8_t *buffer, size_t msglen, size_t structsize, const pb_msgdesc_t *msgtype, unsigned flags, bool assert_success)
{
    bool status;
    pb_istream_t stream;
    size_t initial_alloc_count = get_alloc_count();
    uint8_t *buf2 = malloc_with_check(g_bufsize);
    void *msg = malloc_with_check(structsize);
    alltypes_static_TestExtension extmsg = alltypes_static_TestExtension_init_zero;
    pb_extension_t ext = pb_extension_init_zero;
    assert(msg);

    memset(msg, 0, structsize);
    ext.type = &alltypes_static_TestExtension_testextension;
    ext.dest = &extmsg;
    ext.next = NULL;

    if (msgtype == alltypes_static_AllTypes_fields)
    {
        ((alltypes_static_AllTypes*)msg)->extensions = &ext;
    }
    else if (msgtype == alltypes_pointer_AllTypes_fields)
    {
        ((alltypes_pointer_AllTypes*)msg)->extensions = &ext;
    }

    stream = pb_istream_from_buffer(buffer, msglen);
    status = pb_decode_ex(&stream, msgtype, msg, flags);

    if (status)
    {
        validate_message(msg, structsize, msgtype);
    }

    if (assert_success)
    {
        if (!status) fprintf(stderr, "pb_decode: %s\n", PB_GET_ERROR(&stream));
        assert(status);
    }

    pb_release(msgtype, msg);
    free_with_check(msg);
    free_with_check(buf2);
    assert(get_alloc_count() == initial_alloc_count);

    return status;
}

static bool do_stream_decode(const uint8_t *buffer, size_t msglen, size_t fail_after, size_t structsize, const pb_msgdesc_t *msgtype, bool assert_success)
{
    bool status;
    flakystream_t stream;
    size_t initial_alloc_count = get_alloc_count();
    void *msg = malloc_with_check(structsize);
    assert(msg);

    memset(msg, 0, structsize);
    flakystream_init(&stream, buffer, msglen, fail_after);
    status = pb_decode(&stream.stream, msgtype, msg);

    if (status)
    {
        validate_message(msg, structsize, msgtype);
    }

    if (assert_success)
    {
        if (!status) fprintf(stderr, "pb_decode: %s\n", PB_GET_ERROR(&stream.stream));
        assert(status);
    }

    pb_release(msgtype, msg);
    free_with_check(msg);
    assert(get_alloc_count() == initial_alloc_count);

    return status;
}

static int g_sentinel;

static bool field_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    assert(stream);
    assert(field);
    assert(*arg == &g_sentinel);
    return pb_read(stream, NULL, stream->bytes_left);
}

static bool submsg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    assert(stream);
    assert(field);
    assert(*arg == &g_sentinel);
    return true;
}

bool do_callback_decode(const uint8_t *buffer, size_t msglen, bool assert_success)
{
    bool status;
    pb_istream_t stream;
    size_t initial_alloc_count = get_alloc_count();
    alltypes_callback_AllTypes *msg = malloc_with_check(sizeof(alltypes_callback_AllTypes));
    assert(msg);

    memset(msg, 0, sizeof(alltypes_callback_AllTypes));
    stream = pb_istream_from_buffer(buffer, msglen);

    msg->rep_int32.funcs.decode = &field_callback;
    msg->rep_int32.arg = &g_sentinel;
    msg->rep_string.funcs.decode = &field_callback;
    msg->rep_string.arg = &g_sentinel;
    msg->rep_farray.funcs.decode = &field_callback;
    msg->rep_farray.arg = &g_sentinel;
    msg->req_limits.int64_min.funcs.decode = &field_callback;
    msg->req_limits.int64_min.arg = &g_sentinel;
    msg->cb_oneof.funcs.decode = &submsg_callback;
    msg->cb_oneof.arg = &g_sentinel;

    status = pb_decode(&stream, alltypes_callback_AllTypes_fields, msg);

    if (assert_success)
    {
        if (!status) fprintf(stderr, "pb_decode: %s\n", PB_GET_ERROR(&stream));
        assert(status);
    }

    pb_release(alltypes_callback_AllTypes_fields, msg);
    free_with_check(msg);
    assert(get_alloc_count() == initial_alloc_count);

    return status;
}

void do_roundtrip(const uint8_t *buffer, size_t msglen, size_t structsize, const pb_msgdesc_t *msgtype)
{
    bool status;
    uint32_t checksum2, checksum3;
    size_t msglen2, msglen3;
    uint8_t *buf2 = malloc_with_check(g_bufsize);
    void *msg = malloc_with_check(structsize);

    alltypes_static_TestExtension extmsg = alltypes_static_TestExtension_init_zero;
    pb_extension_t ext = pb_extension_init_zero;
    pb_extension_t **ext_field = NULL;
    ext.type = &alltypes_static_TestExtension_testextension;
    ext.dest = &extmsg;
    ext.next = NULL;

    assert(buf2 && msg);

    if (msgtype == alltypes_static_AllTypes_fields)
    {
        ext_field = &((alltypes_static_AllTypes*)msg)->extensions;
    }
    else if (msgtype == alltypes_pointer_AllTypes_fields)
    {
        ext_field = &((alltypes_pointer_AllTypes*)msg)->extensions;
    }

    {
        pb_istream_t stream = pb_istream_from_buffer(buffer, msglen);
        memset(msg, 0, structsize);
        if (ext_field) *ext_field = &ext;
        status = pb_decode(&stream, msgtype, msg);
        if (!status) fprintf(stderr, "pb_decode: %s\n", PB_GET_ERROR(&stream));
        assert(status);

        validate_message(msg, structsize, msgtype);
    }

    {
        pb_ostream_t stream = pb_ostream_from_buffer(buf2, g_bufsize);
        status = pb_encode(&stream, msgtype, msg);

        if (!status && strcmp(PB_GET_ERROR(&stream), "stream full") != 0)
        {
            fprintf(stderr, "pb_encode: %s\n", PB_GET_ERROR(&stream));
            assert(status);
        }

        msglen2 = stream.bytes_written;
        checksum2 = xor32_checksum(buf2, msglen2);
    }

    pb_release(msgtype, msg);

    if (status)
    {
        pb_istream_t stream = pb_istream_from_buffer(buf2, msglen2);
        memset(msg, 0, structsize);
        if (ext_field) *ext_field = &ext;
        status = pb_decode(&stream, msgtype, msg);
        if (!status) fprintf(stderr, "pb_decode: %s\n", PB_GET_ERROR(&stream));
        assert(status);

        validate_message(msg, structsize, msgtype);
    }

    if (status)
    {
        pb_ostream_t stream = pb_ostream_from_buffer(buf2, g_bufsize);
        status = pb_encode(&stream, msgtype, msg);
        if (!status) fprintf(stderr, "pb_encode: %s\n", PB_GET_ERROR(&stream));
        assert(status);
        msglen3 = stream.bytes_written;
        checksum3 = xor32_checksum(buf2, msglen3);

        assert(msglen2 == msglen3);
        assert(checksum2 == checksum3);
    }

    pb_release(msgtype, msg);
    free_with_check(msg);
    free_with_check(buf2);
}

void do_roundtrips(const uint8_t *data, size_t size, bool expect_valid)
{
    size_t initial_alloc_count = get_alloc_count();
    PB_UNUSED(expect_valid);

#ifdef FUZZTEST_PROTO2_STATIC
    if (do_decode(data, size, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields, 0, expect_valid))
    {
        do_roundtrip(data, size, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields);
        do_stream_decode(data, size, SIZE_MAX, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields, true);
        do_callback_decode(data, size, true);
    }
#endif

#ifdef FUZZTEST_PROTO3_STATIC
    if (do_decode(data, size, sizeof(alltypes_proto3_static_AllTypes), alltypes_proto3_static_AllTypes_fields, 0, expect_valid))
    {
        do_roundtrip(data, size, sizeof(alltypes_proto3_static_AllTypes), alltypes_proto3_static_AllTypes_fields);
        do_stream_decode(data, size, SIZE_MAX, sizeof(alltypes_proto3_static_AllTypes), alltypes_proto3_static_AllTypes_fields, true);
    }
#endif

#ifdef FUZZTEST_PROTO2_POINTER
    if (do_decode(data, size, sizeof(alltypes_pointer_AllTypes), alltypes_pointer_AllTypes_fields, 0, expect_valid))
    {
        do_roundtrip(data, size, sizeof(alltypes_pointer_AllTypes), alltypes_pointer_AllTypes_fields);
        do_stream_decode(data, size, SIZE_MAX, sizeof(alltypes_pointer_AllTypes), alltypes_pointer_AllTypes_fields, true);
    }
#endif

#ifdef FUZZTEST_PROTO3_POINTER
    if (do_decode(data, size, sizeof(alltypes_proto3_pointer_AllTypes), alltypes_proto3_pointer_AllTypes_fields, 0, expect_valid))
    {
        do_roundtrip(data, size, sizeof(alltypes_proto3_pointer_AllTypes), alltypes_proto3_pointer_AllTypes_fields);
        do_stream_decode(data, size, SIZE_MAX, sizeof(alltypes_proto3_pointer_AllTypes), alltypes_proto3_pointer_AllTypes_fields, true);
    }
#endif

#ifdef FUZZTEST_IO_ERRORS
    {
        size_t orig_max_alloc_bytes = get_max_alloc_bytes();

        set_max_alloc_bytes(get_alloc_bytes() + 4096);
        do_stream_decode(data, size, size - 16, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields, false);
        do_stream_decode(data, size, size - 16, sizeof(alltypes_pointer_AllTypes), alltypes_pointer_AllTypes_fields, false);
        set_max_alloc_bytes(orig_max_alloc_bytes);
    }

    do_decode(data, size, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields, PB_DECODE_NOINIT | PB_DECODE_DELIMITED, false);
    do_decode(data, size, sizeof(alltypes_static_AllTypes), alltypes_static_AllTypes_fields, PB_DECODE_NULLTERMINATED, false);

    do_callback_decode(data, size, false);
#endif

    assert(get_alloc_count() == initial_alloc_count);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size > g_bufsize)
        return 0;

    do_roundtrips(data, size, false);

    return 0;
}

#ifndef LLVMFUZZER

static bool generate_base_message(uint8_t *buffer, size_t *msglen)
{
    pb_ostream_t stream;
    bool status;
    static const alltypes_static_AllTypes initval = alltypes_static_AllTypes_init_default;

    alltypes_static_AllTypes *msg = malloc_with_check(sizeof(alltypes_static_AllTypes));
    memcpy(msg, &initval, sizeof(initval));

    while (rand_int(0, 7))
        rand_mess((uint8_t*)msg, sizeof(alltypes_static_AllTypes));

    msg->extensions = NULL;

    stream = pb_ostream_from_buffer(buffer, g_bufsize);
    status = pb_encode(&stream, alltypes_static_AllTypes_fields, msg);
    assert(stream.bytes_written <= g_bufsize);
    assert(stream.bytes_written <= alltypes_static_AllTypes_size);

    *msglen = stream.bytes_written;
    pb_release(alltypes_static_AllTypes_fields, msg);
    free_with_check(msg);

    return status;
}

static void run_iteration()
{
    uint8_t *buffer = malloc_with_check(g_bufsize);
    size_t msglen;

    rand_fill(buffer, g_bufsize);

    if (generate_base_message(buffer, &msglen))
    {
        rand_protobuf_noise(buffer, g_bufsize, &msglen);

        do_roundtrips(buffer, msglen, true);

        while (rand_bool())
            rand_mess(buffer, g_bufsize);

        if (rand_bool())
            msglen = rand_int(0, g_bufsize);

        do_roundtrips(buffer, msglen, false);
    }

    free_with_check(buffer);
    assert(get_alloc_count() == 0);
}

int main(int argc, char **argv)
{
    int i;
    int iterations;

    if (argc >= 2)
    {

        if (g_bufsize > FUZZTEST_MAX_STANDALONE_BUFSIZE)
            g_bufsize = FUZZTEST_MAX_STANDALONE_BUFSIZE;

        random_set_seed(strtoul(argv[1], NULL, 0));
        iterations = (argc >= 3) ? atol(argv[2]) : 10000;

        for (i = 0; i < iterations; i++)
        {
            printf("Iteration %d/%d, seed %lu\n", i, iterations, (unsigned long)random_get_seed());
            run_iteration();
        }
    }
    else
    {

        uint8_t *buffer;
        size_t msglen;

        buffer = malloc_with_check(g_bufsize);

        SET_BINARY_MODE(stdin);
        msglen = fread(buffer, 1, g_bufsize, stdin);
        LLVMFuzzerTestOneInput(buffer, msglen);

        if (!feof(stdin))
        {

            fprintf(stderr, "Warning: input message too long\n");
            while (fread(buffer, 1, g_bufsize, stdin) == g_bufsize);
        }

        free_with_check(buffer);
    }

    return 0;
}
#endif
