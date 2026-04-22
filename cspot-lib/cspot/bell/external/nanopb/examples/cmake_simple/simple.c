#include <stdio.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "simple.pb.h"

int main()
{

    uint8_t buffer[128];
    size_t message_length;
    bool status;

    {

        SimpleMessage message = SimpleMessage_init_zero;

        pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

        message.lucky_number = 13;

        status = pb_encode(&stream, SimpleMessage_fields, &message);
        message_length = stream.bytes_written;

        if (!status)
        {
            printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }
    }

    {

        SimpleMessage message = SimpleMessage_init_zero;

        pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

        status = pb_decode(&stream, SimpleMessage_fields, &message);

        if (!status)
        {
            printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
            return 1;
        }

        printf("Your lucky number was %d!\n", message.lucky_number);
    }

    return 0;
}

