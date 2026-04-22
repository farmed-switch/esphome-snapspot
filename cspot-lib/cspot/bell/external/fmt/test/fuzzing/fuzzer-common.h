

#ifndef FUZZER_COMMON_H
#define FUZZER_COMMON_H

#include <fmt/core.h>

#include <cstdint>
#include <cstring>
#include <vector>

#define FMT_FUZZ_FORMAT_TO_STRING 0

#define FMT_FUZZ_SEPARATE_ALLOCATION 1

constexpr auto fixed_size = 16;

template <typename T> inline const char* as_chars(const T* data) {
  return reinterpret_cast<const char*>(data);
}

template <typename T> inline const std::uint8_t* as_bytes(const T* data) {
  return reinterpret_cast<const std::uint8_t*>(data);
}

template <class Item> inline Item assign_from_buf(const std::uint8_t* data) {
  auto item = Item();
  std::memcpy(&item, data, sizeof(Item));
  return item;
}

template <> inline bool assign_from_buf<bool>(const std::uint8_t* data) {
  return *data != 0;
}

struct data_to_string {
#if FMT_FUZZ_SEPARATE_ALLOCATION
  std::vector<char> buffer;

  data_to_string(const uint8_t* data, size_t size, bool add_terminator = false)
      : buffer(size + (add_terminator ? 1 : 0)) {
    if (size) {
      std::memcpy(buffer.data(), data, size);
    }
  }

  fmt::string_view get() const { return {buffer.data(), buffer.size()}; }
#else
  fmt::string_view sv;

  data_to_string(const uint8_t* data, size_t size, bool = false)
      : str(as_chars(data), size) {}

  fmt::string_view get() const { return sv; }
#endif

  const char* data() const { return get().data(); }
};

#endif
