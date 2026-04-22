

#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <iostream>

static volatile int *Null = 0;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size == 0) {
    std::cout << "Found the target, dereferencing NULL\n";
    *Null = 1;
  }
  return 0;
}

