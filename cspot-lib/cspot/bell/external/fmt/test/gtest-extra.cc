

#include "gtest-extra.h"

#if FMT_USE_FCNTL

using fmt::file;

output_redirect::output_redirect(FILE* f) : file_(f) {
  flush();
  int fd = FMT_POSIX(fileno(f));

  original_ = file::dup(fd);

  file write_end;
  file::pipe(read_end_, write_end);

  write_end.dup2(fd);
}

output_redirect::~output_redirect() noexcept {
  try {
    restore();
  } catch (const std::exception& e) {
    std::fputs(e.what(), stderr);
  }
}

void output_redirect::flush() {
  int result = 0;
  do {
    result = fflush(file_);
  } while (result == EOF && errno == EINTR);
  if (result != 0) throw fmt::system_error(errno, "cannot flush stream");
}

void output_redirect::restore() {
  if (original_.descriptor() == -1) return;
  flush();

  original_.dup2(FMT_POSIX(fileno(file_)));
  original_.close();
}

std::string output_redirect::restore_and_read() {

  restore();

  std::string content;
  if (read_end_.descriptor() == -1) return content;
  enum { BUFFER_SIZE = 4096 };
  char buffer[BUFFER_SIZE];
  size_t count = 0;
  do {
    count = read_end_.read(buffer, BUFFER_SIZE);
    content.append(buffer, count);
  } while (count != 0);
  read_end_.close();
  return content;
}

std::string read(file& f, size_t count) {
  std::string buffer(count, '\0');
  size_t n = 0, offset = 0;
  do {
    n = f.read(&buffer[offset], count - offset);

    offset += n;
  } while (offset < count && n != 0);
  buffer.resize(offset);
  return buffer;
}

#endif
