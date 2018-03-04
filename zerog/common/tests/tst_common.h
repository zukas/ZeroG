#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "common/bitop.h"

using namespace testing;

TEST(bitop, single_set_unset_pass) {
  uint64_t mask{};
  mask = set_mask(mask, 0);
  EXPECT_EQ(test_mask(mask, 0), true);
  mask = unset_mask(mask, 0);
  EXPECT_EQ(test_mask(mask, 0), false);
}

#ifndef NDEBUG
TEST(bitop, single_set_overflow) {
  uint64_t mask{};
  EXPECT_DEATH(
      mask = set_mask(mask, 66),
      "Assertion `bit_pos < sizeof64 && \"Bit number exceeded 64\"' failed.");
}

TEST(bitop, single_test_overflow) {
  uint64_t mask{};
  EXPECT_DEATH(
      test_mask(mask, 66),
      "Assertion `bit_pos < sizeof64 && \"Bit number exceeded 64\"' failed.");
}

TEST(bitop, single_unset_overflow) {
  uint64_t mask{};
  EXPECT_DEATH(
      mask = unset_mask(mask, 66),
      "Assertion `bit_pos < sizeof64 && \"Bit number exceeded 64\"' failed.");
}
#endif

TEST(bitop, single_set_unset_bad_bit) {
  uint64_t mask{};
  mask = set_mask(mask, 12);
  EXPECT_EQ(test_mask(mask, 12), true);
  mask = unset_mask(mask, 11);
  EXPECT_EQ(test_mask(mask, 12), true);
}

TEST(bitop, multi_set_unset_pass) {
  uint64_t mask{};
  mask = set_mask(mask, 0, 3);
  EXPECT_EQ(test_mask(mask, 0, 3), true);
  mask = unset_mask(mask, 0, 3);
  EXPECT_EQ(test_mask(mask, 0, 3), false);
}

TEST(bitop, multi_set_unset_large_pass) {
  uint64_t mask{};
  mask = set_mask(mask, 1, 62);
  EXPECT_EQ(test_mask(mask, 1, 62), true);
  mask = unset_mask(mask, 1, 62);
  EXPECT_EQ(test_mask(mask, 1, 62), false);
}

TEST(bitop, multi_set_unset_bad_bitset) {
  uint64_t mask{};
  mask = set_mask(mask, 1, 62);
  EXPECT_EQ(test_mask(mask, 0, 4), true);
  mask = unset_mask(mask, 1, 62);
  EXPECT_EQ(test_mask(mask, 0, 63), false);
}

TEST(bitop, multi_set_unset_bitset_complete) {
  uint64_t mask{};
  mask = set_mask(mask, 0, 8);
  EXPECT_EQ(test_mask(mask, 0, 8), true);
  EXPECT_EQ(test_mask(mask, 1, 9), true);
  EXPECT_EQ(test_mask(mask, 2, 10), true);
  EXPECT_EQ(test_mask(mask, 3, 11), true);
  EXPECT_EQ(test_mask(mask, 4, 12), true);
  EXPECT_EQ(test_mask(mask, 5, 13), true);
  EXPECT_EQ(test_mask(mask, 6, 14), true);
  EXPECT_EQ(test_mask(mask, 7, 15), true);
  EXPECT_EQ(test_mask(mask, 8, 16), false);
  mask = unset_mask(mask, 0, 4);
  EXPECT_EQ(test_mask(mask, 0, 4), false);
  EXPECT_EQ(test_mask(mask, 1, 5), true);
  EXPECT_EQ(test_mask(mask, 2, 6), true);
  EXPECT_EQ(test_mask(mask, 3, 7), true);
  EXPECT_EQ(test_mask(mask, 4, 8), true);
  EXPECT_EQ(test_mask(mask, 5, 9), true);
  EXPECT_EQ(test_mask(mask, 6, 10), true);
  EXPECT_EQ(test_mask(mask, 7, 11), true);
}

#ifndef NDEBUG

TEST(bitop, multi_set_overflow_start_bit) {
  uint64_t mask{};
  EXPECT_DEATH(set_mask(mask, 64, 64), "Assertion `start_bit < sizeof64 && "
                                       "\"Start bit number exceeded 64\"' "
                                       "failed.");
}

TEST(bitop, multi_set_overflow_length) {
  uint64_t mask{};
  EXPECT_DEATH(
      set_mask(mask, 0, 66),
      "Assertion `bit_len_n < sizeof64 && \"Bit length exceeded 64\"' failed.");
}

TEST(bitop, multi_set_overflow_size) {
  uint64_t mask{};
  EXPECT_DEATH(set_mask(mask, 10, 60),
               "Assertion `start_bit \\+ bit_len_n < sizeof64 && \"Bit "
               "start \\+ length exceeded 64\"' failed.");
}

#endif
