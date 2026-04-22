#ifndef UTILS_H
#define UTILS_H
#include <cstdio>
#include <vector>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#include "win32shim.h"
#else

#endif
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

#define HMAC_SHA1_BLOCKSIZE 64

unsigned long long getCurrentTimestamp();

uint64_t hton64(uint64_t value);

std::vector<uint8_t> bigNumDivide(std::vector<uint8_t> num, int n);

std::vector<uint8_t> bigNumMultiply(std::vector<uint8_t> num, int n);

std::vector<uint8_t> bigNumAdd(std::vector<uint8_t> num, int n);

unsigned char h2int(char c);

std::string urlDecode(std::string str);

std::vector<uint8_t> stringHexToBytes(const std::string& s);

std::string bytesToHexString(const std::vector<uint8_t>& bytes);

template <typename T>
T extract(const std::vector<unsigned char>& v, int pos) {
  T value;
  memcpy(&value, &v[pos], sizeof(T));
  return value;
}

template <typename T>
std::vector<uint8_t> pack(T data) {
  std::vector<std::uint8_t> rawData((std::uint8_t*)&data,
                                    (std::uint8_t*)&(data) + sizeof(T));

  return rawData;
}

template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
  int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) +
               1;
  if (size_s <= 0) {
    throw std::runtime_error("Error during formatting.");
  }
  auto size = static_cast<size_t>(size_s);
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args...);
  return std::string(buf.get(),
                     buf.get() + size - 1);
}

#endif