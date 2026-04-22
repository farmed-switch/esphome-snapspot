#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <vector>

namespace bell {
class ByteStream;

class BinaryReader {
  std::shared_ptr<ByteStream> stream;
  size_t currentPos = 0;

 public:
  BinaryReader(std::shared_ptr<ByteStream> stream);
  int32_t readInt();
  int16_t readShort();
  uint32_t readUInt();
  long long readLong();
  void close();
  uint8_t readByte();
  size_t size();
  size_t position();
  std::vector<uint8_t> readBytes(size_t);
  void skip(size_t);
};
}