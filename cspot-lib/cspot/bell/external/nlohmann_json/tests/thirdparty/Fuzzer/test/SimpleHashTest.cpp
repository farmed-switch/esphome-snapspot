

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

static uint32_t simple_hash(const uint8_t *Data, size_t Size) {
  uint32_t Hash = 0x12039854;
  for (uint32_t i = 0; i < Size; i++) {
    Hash += Data[i];
    Hash += (Hash << 10);
    Hash ^= (Hash >> 6);
  }
  Hash += (Hash << 3);
  Hash ^= (Hash >> 11);
  Hash += (Hash << 15);
  return Hash;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  if (Size < 14)
    return 0;

  uint32_t Hash = simple_hash(&Data[0], Size - 4);
  uint32_t Want = reinterpret_cast<const uint32_t *>(&Data[Size - 4])[0];
  if (Hash != Want)
    return 0;
  fprintf(stderr, "BINGO; simple_hash defeated: %x == %x\n", (unsigned int)Hash,
          (unsigned int)Want);
  exit(1);
  return 0;
}
