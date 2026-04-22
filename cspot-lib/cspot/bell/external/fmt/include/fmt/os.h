

#ifndef FMT_OS_H_
#define FMT_OS_H_

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <system_error>

#if defined __APPLE__ || defined(__FreeBSD__)
#  include <xlocale.h>
#endif

#include "format.h"

#ifndef FMT_USE_FCNTL

#  if FMT_HAS_INCLUDE("winapifamily.h")
#    include <winapifamily.h>
#  endif
#  if (FMT_HAS_INCLUDE(<fcntl.h>) || defined(__APPLE__) || \
       defined(__linux__)) &&                              \
      (!defined(WINAPI_FAMILY) ||                          \
       (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
#    include <fcntl.h>
#    define FMT_USE_FCNTL 1
#  else
#    define FMT_USE_FCNTL 0
#  endif
#endif

#ifndef FMT_POSIX
#  if defined(_WIN32) && !defined(__MINGW32__)

#    define FMT_POSIX(call) _##call
#  else
#    define FMT_POSIX(call) call
#  endif
#endif

#ifdef FMT_SYSTEM
#  define FMT_POSIX_CALL(call) FMT_SYSTEM(call)
#else
#  define FMT_SYSTEM(call) ::call
#  ifdef _WIN32

#    define FMT_POSIX_CALL(call) ::_##call
#  else
#    define FMT_POSIX_CALL(call) ::call
#  endif
#endif

#ifndef _WIN32
#  define FMT_RETRY_VAL(result, expression, error_result) \
    do {                                                  \
      (result) = (expression);                            \
    } while ((result) == (error_result) && errno == EINTR)
#else
#  define FMT_RETRY_VAL(result, expression, error_result) result = (expression)
#endif

#define FMT_RETRY(result, expression) FMT_RETRY_VAL(result, expression, -1)

FMT_BEGIN_NAMESPACE
FMT_MODULE_EXPORT_BEGIN

template <typename Char> class basic_cstring_view {
 private:
  const Char* data_;

 public:

  basic_cstring_view(const Char* s) : data_(s) {}

  basic_cstring_view(const std::basic_string<Char>& s) : data_(s.c_str()) {}

  const Char* c_str() const { return data_; }
};

using cstring_view = basic_cstring_view<char>;
using wcstring_view = basic_cstring_view<wchar_t>;

template <typename Char> struct formatter<std::error_code, Char> {
  template <typename ParseContext>
  FMT_CONSTEXPR auto parse(ParseContext& ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }

  template <typename FormatContext>
  FMT_CONSTEXPR auto format(const std::error_code& ec, FormatContext& ctx) const
      -> decltype(ctx.out()) {
    auto out = ctx.out();
    out = detail::write_bytes(out, ec.category().name(),
                              basic_format_specs<Char>());
    out = detail::write<Char>(out, Char(':'));
    out = detail::write<Char>(out, ec.value());
    return out;
  }
};

#ifdef _WIN32
FMT_API const std::error_category& system_category() noexcept;

FMT_BEGIN_DETAIL_NAMESPACE

class utf16_to_utf8 {
 private:
  memory_buffer buffer_;

 public:
  utf16_to_utf8() {}
  FMT_API explicit utf16_to_utf8(basic_string_view<wchar_t> s);
  operator string_view() const { return string_view(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const char* c_str() const { return &buffer_[0]; }
  std::string str() const { return std::string(&buffer_[0], size()); }

  FMT_API int convert(basic_string_view<wchar_t> s);
};

FMT_API void format_windows_error(buffer<char>& out, int error_code,
                                  const char* message) noexcept;
FMT_END_DETAIL_NAMESPACE

FMT_API std::system_error vwindows_error(int error_code, string_view format_str,
                                         format_args args);

template <typename... Args>
std::system_error windows_error(int error_code, string_view message,
                                const Args&... args) {
  return vwindows_error(error_code, message, fmt::make_format_args(args...));
}

FMT_API void report_windows_error(int error_code, const char* message) noexcept;
#else
inline const std::error_category& system_category() noexcept {
  return std::system_category();
}
#endif

#ifdef __OSX__
template <typename S, typename... Args, typename Char = char_t<S>>
void say(const S& format_str, Args&&... args) {
  std::system(format("say \"{}\"", format(format_str, args...)).c_str());
}
#endif

class buffered_file {
 private:
  FILE* file_;

  friend class file;

  explicit buffered_file(FILE* f) : file_(f) {}

 public:
  buffered_file(const buffered_file&) = delete;
  void operator=(const buffered_file&) = delete;

  buffered_file() noexcept : file_(nullptr) {}

  FMT_API ~buffered_file() noexcept;

 public:
  buffered_file(buffered_file&& other) noexcept : file_(other.file_) {
    other.file_ = nullptr;
  }

  buffered_file& operator=(buffered_file&& other) {
    close();
    file_ = other.file_;
    other.file_ = nullptr;
    return *this;
  }

  FMT_API buffered_file(cstring_view filename, cstring_view mode);

  FMT_API void close();

  FILE* get() const noexcept { return file_; }

  FMT_API int descriptor() const;

  void vprint(string_view format_str, format_args args) {
    fmt::vprint(file_, format_str, args);
  }

  template <typename... Args>
  inline void print(string_view format_str, const Args&... args) {
    vprint(format_str, fmt::make_format_args(args...));
  }
};

#if FMT_USE_FCNTL

class FMT_API file {
 private:
  int fd_;

  explicit file(int fd) : fd_(fd) {}

 public:

  enum {
    RDONLY = FMT_POSIX(O_RDONLY),
    WRONLY = FMT_POSIX(O_WRONLY),
    RDWR = FMT_POSIX(O_RDWR),
    CREATE = FMT_POSIX(O_CREAT),
    APPEND = FMT_POSIX(O_APPEND),
    TRUNC = FMT_POSIX(O_TRUNC)
  };

  file() noexcept : fd_(-1) {}

  file(cstring_view path, int oflag);

 public:
  file(const file&) = delete;
  void operator=(const file&) = delete;

  file(file&& other) noexcept : fd_(other.fd_) { other.fd_ = -1; }

  file& operator=(file&& other) {
    close();
    fd_ = other.fd_;
    other.fd_ = -1;
    return *this;
  }

  ~file() noexcept;

  int descriptor() const noexcept { return fd_; }

  void close();

  long long size() const;

  size_t read(void* buffer, size_t count);

  size_t write(const void* buffer, size_t count);

  static file dup(int fd);

  void dup2(int fd);

  void dup2(int fd, std::error_code& ec) noexcept;

  static void pipe(file& read_end, file& write_end);

  buffered_file fdopen(const char* mode);
};

long getpagesize();

FMT_BEGIN_DETAIL_NAMESPACE

struct buffer_size {
  buffer_size() = default;
  size_t value = 0;
  buffer_size operator=(size_t val) const {
    auto bs = buffer_size();
    bs.value = val;
    return bs;
  }
};

struct ostream_params {
  int oflag = file::WRONLY | file::CREATE | file::TRUNC;
  size_t buffer_size = BUFSIZ > 32768 ? BUFSIZ : 32768;

  ostream_params() {}

  template <typename... T>
  ostream_params(T... params, int new_oflag) : ostream_params(params...) {
    oflag = new_oflag;
  }

  template <typename... T>
  ostream_params(T... params, detail::buffer_size bs)
      : ostream_params(params...) {
    this->buffer_size = bs.value;
  }

#  if defined(__INTEL_COMPILER) && __INTEL_COMPILER < 2000
  ostream_params(int new_oflag) : oflag(new_oflag) {}
  ostream_params(detail::buffer_size bs) : buffer_size(bs.value) {}
#  endif
};

FMT_END_DETAIL_NAMESPACE

constexpr detail::buffer_size buffer_size{};

class FMT_API ostream final : private detail::buffer<char> {
 private:
  file file_;

  void grow(size_t) override;

  ostream(cstring_view path, const detail::ostream_params& params)
      : file_(path, params.oflag) {
    set(new char[params.buffer_size], params.buffer_size);
  }

 public:
  ostream(ostream&& other)
      : detail::buffer<char>(other.data(), other.size(), other.capacity()),
        file_(std::move(other.file_)) {
    other.clear();
    other.set(nullptr, 0);
  }
  ~ostream() {
    flush();
    delete[] data();
  }

  void flush() {
    if (size() == 0) return;
    file_.write(data(), size());
    clear();
  }

  template <typename... T>
  friend ostream output_file(cstring_view path, T... params);

  void close() {
    flush();
    file_.close();
  }

  template <typename... T> void print(format_string<T...> fmt, T&&... args) {
    vformat_to(detail::buffer_appender<char>(*this), fmt,
               fmt::make_format_args(args...));
  }
};

template <typename... T>
inline ostream output_file(cstring_view path, T... params) {
  return {path, detail::ostream_params(params...)};
}
#endif

FMT_MODULE_EXPORT_END
FMT_END_NAMESPACE

#endif
