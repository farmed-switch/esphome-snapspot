

#include <stdio.h>
#include <pb_encode.h>
#include "person.pb.h"
#include "test_helpers.h"

int main()
{
    uint8_t buffer[Person_size];
    pb_ostream_t stream;

    Person person = {"Test Person 99", 99, true, "test@person.com",
        3, {{"555-12345678", true, Person_PhoneType_MOBILE},
            {"99-2342", false, 0},
            {"1234-5678", true, Person_PhoneType_WORK},
        }};

    stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (pb_encode(&stream, Person_fields, &person))
    {

        SET_BINARY_MODE(stdout);
        fwrite(buffer, 1, stream.bytes_written, stdout);
        return 0;
    }
    else
    {
        fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&stream));
        return 1;
    }
}
