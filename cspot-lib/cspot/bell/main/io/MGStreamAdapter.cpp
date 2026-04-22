
#include "MGStreamAdapter.h"

mg_buf::mg_buf(struct mg_connection* _conn) : conn(_conn) {
  setp(buffer, buffer + BUF_SIZE - 1);
}

mg_buf::int_type mg_buf::overflow(int_type c) {
  if (c != EOF) {
    *pptr() = c;
    pbump(1);
  }

  if (flush_buffer() == EOF) {
    return EOF;
  }

  return c;
}

int mg_buf::flush_buffer() {
  int len = int(pptr() - pbase());
  if (mg_write(conn, buffer, len) != len) {
    return EOF;
  }
  pbump(-len);
  return len;
}

int mg_buf::sync() {
  if (flush_buffer() == EOF) {
    return -1;
  }
  return 0;
}

MGStreamAdapter::MGStreamAdapter(struct mg_connection* _conn)
    : std::ostream(&buf), buf(_conn) {
  rdbuf(&buf);
}

mg_read_buf::mg_read_buf(struct mg_connection* _conn) : conn(_conn) {
  setg(buffer + BUF_SIZE,
       buffer + BUF_SIZE,
       buffer + BUF_SIZE);
}

mg_read_buf::int_type mg_read_buf::underflow() {
  if (gptr() < egptr()) {
    return traits_type::to_int_type(*gptr());
  }

  char* base = buffer;
  char* start = base;

  if (eback() == base) {

    std::memmove(base, egptr() - 2, 2);
    start += 2;
  }

  int n = mg_read(conn, start, buffer + BUF_SIZE - start);
  if (n == 0) {
    return traits_type::eof();
  }

  setg(base, start, start + n);

  return traits_type::to_int_type(*gptr());
}

MGInputStreamAdapter::MGInputStreamAdapter(struct mg_connection* _conn)
    : std::istream(&buf), buf(_conn) {
  rdbuf(&buf);
}
