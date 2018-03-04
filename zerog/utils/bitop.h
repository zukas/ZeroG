#ifndef BITOP_H
#define BITOP_H

#include <cassert>
#include <cstdint>

constexpr uint64_t set_mask(uint64_t mask, uint64_t bit) {
  assert(bit < 64);
  return mask | (1lu << bit);
}

constexpr uint64_t unset_mask(uint64_t mask, uint64_t bit) {
  assert(bit < 64);
  return mask | (0lu << bit);
}

constexpr bool test_mask(uint64_t mask, uint64_t bit) {
  assert(bit < 64);
  return !!((1lu << bit) & mask);
}

constexpr uint64_t max(uint64_t x, uint64_t y) {
  return x ^ ((x ^ y) & -(x < y));
}

#endif // BITOP_H
