

#include <assert.h>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <iostream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  assert(Data);

  size_t CurA = 0, MaxA = 0;
  for (size_t i = 0; i < Size; i++) {

    int EQ = Data[i] == 'A';
    CurA = EQ * (CurA + 1);
    int GT = CurA > MaxA;
    MaxA = GT * CurA + (!GT) * MaxA;
  }
  if (MaxA >= 20) {
    std::cout << "BINGO; Found the target (Max: " << MaxA << "), exiting\n";
    exit(0);
  }
  return 0;
}

