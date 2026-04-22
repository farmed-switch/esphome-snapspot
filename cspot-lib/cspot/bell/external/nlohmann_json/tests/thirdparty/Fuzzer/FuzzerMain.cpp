

#include "FuzzerDefs.h"

extern "C" {

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size);
}

int main(int argc, char **argv) {
  return fuzzer::FuzzerDriver(&argc, &argv, LLVMFuzzerTestOneInput);
}
