#pragma once

#include <ByteStream.h>
#include <stdint.h>
#include <stdio.h>
#include <string>

namespace bell {
class FileStream : public ByteStream {
 public:
  FileStream(const std::string& path, std::string mode);
  ~FileStream();

  FILE* file;

  size_t read(uint8_t* buf, size_t nbytes);

  size_t skip(size_t nbytes);

  size_t position();

  size_t size();

  void close();
};
}
