

#include <cstdint>
#include <cstddef>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  static volatile int Zero = 0;
  if (!Size)
    while(!Zero)
      ;
  return 0;
}
