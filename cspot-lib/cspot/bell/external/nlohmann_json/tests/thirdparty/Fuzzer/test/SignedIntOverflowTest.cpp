

#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <climits>

static volatile int Sink;
static int Large = INT_MAX;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  assert(Data);
  if (Size > 0 && Data[0] == 'H') {
    Sink = 1;
    if (Size > 1 && Data[1] == 'i') {
      Sink = 2;
      if (Size > 2 && Data[2] == '!') {
        Large++;
      }
    }
  }
  return 0;
}

