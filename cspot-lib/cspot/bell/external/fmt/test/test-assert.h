

#ifndef FMT_TEST_ASSERT_H_
#define FMT_TEST_ASSERT_H_

#include <stdexcept>

void throw_assertion_failure(const char* message);
#define FMT_ASSERT(condition, message) \
  if (!(condition)) throw_assertion_failure(message);

#include "gtest/gtest.h"

class assertion_failure : public std::logic_error {
 public:
  explicit assertion_failure(const char* message) : std::logic_error(message) {}

 private:
  virtual void avoid_weak_vtable();
};

void assertion_failure::avoid_weak_vtable() {}

inline void throw_assertion_failure(const char* message) {
  throw assertion_failure(message);
}

#define EXPECT_ASSERT(stmt, message) \
  FMT_TEST_THROW_(stmt, assertion_failure, message, GTEST_NONFATAL_FAILURE_)

#endif
