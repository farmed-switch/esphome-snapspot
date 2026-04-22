#include <pb_encode.h>
#include <unittests.h>
#include <string.h>
#include "padding.pb.h"

int main()
{
    int status = 0;

    TestMessage msg;

    memset(&msg, 0xAA, sizeof(msg));

    msg.submsg.boolfield = false;
    msg.submsg.intfield = 0;

    {
        pb_byte_t buf[128] = {0};
        pb_ostream_t stream = pb_ostream_from_buffer(buf, sizeof(buf));
        TEST(pb_encode(&stream, TestMessage_fields, &msg));

        TEST(stream.bytes_written == 0);
    }

    return status;
}

