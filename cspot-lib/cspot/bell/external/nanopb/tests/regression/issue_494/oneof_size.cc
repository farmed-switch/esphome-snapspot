#include "oneof.pb.h"
#include "unittests.h"

extern "C" int main()
{
    int status = 0;

    printf("Size: %d\n", (int)MyMessage_size);
    TEST(MyMessage_size == 18);

    return status;
}
