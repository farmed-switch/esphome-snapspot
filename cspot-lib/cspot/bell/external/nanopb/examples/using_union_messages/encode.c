

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pb_encode.h>
#include <pb_common.h>
#include "unionproto.pb.h"

bool encode_unionmessage(pb_ostream_t *stream, const pb_msgdesc_t *messagetype, void *message)
{
    pb_field_iter_t iter;

    if (!pb_field_iter_begin(&iter, UnionMessage_fields, message))
        return false;

    do
    {
        if (iter.submsg_desc == messagetype)
        {

            if (!pb_encode_tag_for_field(stream, &iter))
                return false;

            return pb_encode_submessage(stream, messagetype, message);
        }
    } while (pb_field_iter_next(&iter));

    return false;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s (1|2|3)\n", argv[0]);
        return 1;
    }

    uint8_t buffer[512];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    bool status = false;
    int msgtype = atoi(argv[1]);
    if (msgtype == 1)
    {

        MsgType1 msg = {42};
        status = encode_unionmessage(&stream, MsgType1_fields, &msg);
    }
    else if (msgtype == 2)
    {

        MsgType2 msg = {true};
        status = encode_unionmessage(&stream, MsgType2_fields, &msg);
    }
    else if (msgtype == 3)
    {

        MsgType3 msg = {3, 1415};
        status = encode_unionmessage(&stream, MsgType3_fields, &msg);
    }
    else
    {
        fprintf(stderr, "Unknown message type: %d\n", msgtype);
        return 2;
    }

    if (!status)
    {
        fprintf(stderr, "Encoding failed!\n");
        return 3;
    }
    else
    {
        fwrite(buffer, 1, stream.bytes_written, stdout);
        return 0;
    }
}

