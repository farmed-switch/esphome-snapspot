

#include <vector>

#include "fmt/ranges.h"
#include "gtest/gtest.h"

TEST(ranges_odr_test, format_vector) {
  auto v = std::vector<int>{1, 2, 3, 5, 7, 11};
  EXPECT_EQ(fmt::format("{}", v), "[1, 2, 3, 5, 7, 11]");
}
