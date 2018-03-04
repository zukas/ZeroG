#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace ZeroG {
template <typename __T> struct array {
  __T *data;
  size_t size;
  typedef __T *iterator;
  typedef const __T *const_iterator;

  iterator begin() { return &data[0]; }
  const_iterator begin() const { return &data[0]; }
  iterator end() { return &data[size]; }
  const_iterator end() const { return &data[size]; }
};
template <typename __T> static array<__T> createArray(size_t size) {
  return array<__T>{static_cast<__T *>(malloc(sizeof(__T) * size)), size};
}
template <typename __T> static void destroyArray(array<__T> data) {
  free(data.data);
}
} // namespace ZeroG

#endif // ARRAY_H
