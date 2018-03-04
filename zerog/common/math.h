#ifndef MATH_H
#define MATH_H

#include <cstdint>

#define bmin(x, y) (y ^ ((x ^ y) & -(x < y)))
#define bmax(x, y) (x ^ ((x ^ y) & -(x < y)))

constexpr int32_t min(int32_t x, int32_t y) { return bmin(x, y); }
constexpr uint32_t min(uint32_t x, uint32_t y) { return bmin(x, y); }
constexpr int64_t min(int64_t x, int64_t y) { return bmin(x, y); }
constexpr uint64_t min(uint64_t x, uint64_t y) { return bmin(x, y); }

constexpr int32_t max(int32_t x, int32_t y) { return bmax(x, y); }
constexpr uint32_t max(uint32_t x, uint32_t y) { return bmax(x, y); }
constexpr int64_t max(int64_t x, int64_t y) { return bmax(x, y); }
constexpr uint64_t max(uint64_t x, uint64_t y) { return bmax(x, y); }

#endif // MATH_H
