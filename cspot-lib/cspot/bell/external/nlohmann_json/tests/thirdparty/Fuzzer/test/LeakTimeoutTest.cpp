

#include <cstdint>
#include <cstddef>

static volatile int *Sink;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (!Size) return 0;
  Sink = new int;
  Sink = new int;
  while (Sink) *Sink = 0;
  return 0;
}

