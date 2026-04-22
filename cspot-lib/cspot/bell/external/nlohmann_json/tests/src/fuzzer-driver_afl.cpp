

#include <vector>
#include <cstdint>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

int main()
{
#ifdef __AFL_HAVE_MANUAL_CONTROL
    while (__AFL_LOOP(1000))
    {
#endif

        std::vector<uint8_t> vec;
        char c = 0;
        while (std::cin.get(c))
        {
            vec.push_back(static_cast<uint8_t>(c));
        }

        LLVMFuzzerTestOneInput(vec.data(), vec.size());
#ifdef __AFL_HAVE_MANUAL_CONTROL
    }
#endif
}
