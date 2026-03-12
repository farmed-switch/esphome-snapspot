#pragma once
#include <cstddef>
namespace bell {
class CircularBuffer {
 public:
  explicit CircularBuffer(size_t capacity) {}
  virtual ~CircularBuffer() = default;
  size_t size() const { return 0; }
  size_t capacity() const { return 0; }
};
}
