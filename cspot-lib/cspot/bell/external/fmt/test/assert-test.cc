

#include "fmt/core.h"
#include "gtest/gtest.h"

TEST(assert_test, fail) {
#if GTEST_HAS_DEATH_TEST
  EXPECT_DEBUG_DEATH(FMT_ASSERT(false, "don't panic!"), "don't panic!");
#else
  fmt::print("warning: death tests are not supported\n");
#endif
}

TEST(assert_test, dangling_else) {
  bool test_condition = false;
  bool executed_else = false;
  if (test_condition)
    FMT_ASSERT(true, "");
  else
    executed_else = true;
  EXPECT_TRUE(executed_else);
}
