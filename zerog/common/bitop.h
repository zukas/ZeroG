#ifndef BITOP_H
#define BITOP_H

#include <cassert>
#include <climits>
#include <cmath>
#include <cstdint>

constexpr int32_t sizeof64{CHAR_BIT * sizeof(uint64_t)};

constexpr uint64_t set_mask(uint64_t mask, int32_t bit_pos) {
  assert(bit_pos < sizeof64 && "Bit number exceeded 64");
  return mask | (1lu << bit_pos);
}

constexpr uint64_t set_mask(uint64_t mask, int32_t start_bit, int32_t bit_len) {
  const int32_t bit_len_n = (bit_len - 1);
  assert(start_bit < sizeof64 && "Start bit number exceeded 64");
  assert(bit_len_n < sizeof64 && "Bit length exceeded 64");
  assert(start_bit + bit_len_n < sizeof64 && "Bit start + length exceeded 64");
  const uint64_t add = ((2ul << bit_len_n) - 1ul) << start_bit;
  return mask | add;
}

constexpr uint64_t unset_mask(uint64_t mask, int32_t bit_pos) {
  assert(bit_pos < sizeof64 && "Bit number exceeded 64");
  return mask & ~(1lu << bit_pos);
}

constexpr uint64_t unset_mask(uint64_t mask, int32_t start_bit,
                              int32_t bit_len) {
  const int32_t bit_len_n = (bit_len - 1);
  assert(start_bit < sizeof64 && "Start bit number exceeded 64");
  assert(bit_len_n < sizeof64 && "Bit length exceeded 64");
  assert((start_bit + bit_len_n) < sizeof64 &&
         "Bit start + length exceeded 64");
  const uint64_t rem = ((2ul << bit_len_n) - 1ul) << start_bit;
  return mask & ~rem;
}

constexpr bool test_mask(uint64_t mask, int32_t bit_pos) {
  assert(bit_pos < sizeof64 && "Bit number exceeded 64");
  return !!((1lu << bit_pos) & mask);
}

constexpr bool test_mask(uint64_t mask, int32_t start_bit, int32_t bit_len) {
  const int32_t bit_len_n = (bit_len - 1);
  assert(start_bit < sizeof64 && "Start bit number exceeded 64");
  assert(bit_len_n < sizeof64 && "Bit length exceeded 64");
  assert((start_bit + bit_len_n) < sizeof64 &&
         "Bit start + length exceeded 64");
  const uint64_t test = ((2ul << bit_len_n) - 1ul) << start_bit;
  return (mask & (test)) != 0;
}

int32_t find_mask_bits(uint64_t mask, int32_t bit_len) {

  const int32_t bit_len_n = (bit_len - 1);
  assert(bit_len_n < sizeof64 && "Bit length exceeded 64");
  const int32_t bit_end = sizeof64 - bit_len_n;
  const uint64_t test = ((2ul << bit_len_n) - 1ul);

  int32_t idx = -1;
  int32_t i = 0;
  while (i < bit_end) {
    uint64_t res = (mask & (test << i));
    if (res == 0) {
      idx = i;
      break;
    }
    /**
     * @brief Built-in Function: int __builtin_clzl (unsigned long)
     *
     * @return the number of leading 0-bits in x, starting at the most
     * significant bit position.
     * @warning If x is 0, the result is undefined.
     **/
    i = sizeof64 - __builtin_clzl(res);
  }
  return idx;
}

#endif // BITOP_H
