

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pb_decode.h>
#include <assert.h>
#include "oneof.pb.h"
#include "test_helpers.h"
#include "unittests.h"

bool SubMsg3_callback(pb_istream_t *istream, pb_ostream_t *ostream, const pb_field_t *field)
{
    if (istream && field->tag == SubMsg3_strvalue_tag)
    {

        uint8_t buffer[64];
        int strlen = istream->bytes_left;

        if (strlen > sizeof(buffer) - 1)
            return false;

        buffer[strlen] = '\0';

        if (!pb_read(istream, buffer, strlen))
            return false;

        printf("  strvalue: \"%s\"\n", buffer);
    }

    return true;
}

bool print_int32(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    uint64_t value;
    if (!pb_decode_varint(stream, &value))
        return false;

    printf((char*)*arg, (int)value);
    return true;
}

bool print_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
    uint8_t buffer[64];
    int strlen = stream->bytes_left;

    if (strlen > sizeof(buffer) - 1)
        return false;

    buffer[strlen] = '\0';

    if (!pb_read(stream, buffer, strlen))
        return false;

    printf((char*)*arg, buffer);
    return true;
}

bool msg_callback(pb_istream_t *stream, const pb_field_t *field, void **arg)
{

    OneOfMessage *topmsg = field->message;
    printf("prefix: %d\n", (int)topmsg->prefix);

    if (field->tag == OneOfMessage_submsg1_tag)
    {
        SubMsg1 *msg = field->pData;
        printf("submsg1 {\n");
        msg->array.funcs.decode = print_int32;
        msg->array.arg = "  array: %d\n";
    }
    else if (field->tag == OneOfMessage_submsg2_tag)
    {
        SubMsg2 *msg = field->pData;
        printf("submsg2 {\n");
        msg->strvalue.funcs.decode = print_string;
        msg->strvalue.arg = "  strvalue: \"%s\"\n";
    }
    else if (field->tag == OneOfMessage_submsg3_tag)
    {

        printf("submsg3 {\n");
    }

    return true;
}

int main(int argc, char **argv)
{
    uint8_t buffer[256];
    OneOfMessage msg = OneOfMessage_init_zero;
    pb_istream_t stream;
    size_t count;

    SET_BINARY_MODE(stdin);
    count = fread(buffer, 1, sizeof(buffer), stdin);

    if (!feof(stdin))
    {
        fprintf(stderr, "Message does not fit in buffer\n");
        return 1;
    }

    msg.cb_values.funcs.decode = msg_callback;
    stream = pb_istream_from_buffer(buffer, count);
    if (!pb_decode(&stream, OneOfMessage_fields, &msg))
    {
        fprintf(stderr, "Decoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }

    if (msg.which_values == OneOfMessage_intvalue_tag)
    {
        printf("prefix: %d\n", (int)msg.prefix);
        printf("intvalue: %d\n", (int)msg.values.intvalue);
    }
    else if (msg.which_values == OneOfMessage_strvalue_tag)
    {
        printf("prefix: %d\n", (int)msg.prefix);
        printf("strvalue: \"%s\"\n", msg.values.strvalue);
    }
    else if (msg.which_values == OneOfMessage_submsg3_tag &&
             msg.values.submsg3.which_values == SubMsg3_intvalue_tag)
    {
        printf("  intvalue: %d\n", (int)msg.values.submsg3.values.intvalue);
        printf("}\n");
    }
    else
    {
        printf("}\n");
    }
    printf("suffix: %d\n", (int)msg.suffix);

    assert(msg.prefix == 123);
    assert(msg.suffix == 321);

    return 0;
}
