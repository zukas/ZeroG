#ifndef MEMZ2_H
#define MEMZ2_H

#include <cstddef>
#include <cstdint>

namespace memz2 {
typedef struct Allocator_T *Allocator;
enum class AllocatorType : size_t { STACK, POOL, BLOCK };
struct AllocatorCreateInfo {
  AllocatorType type;
  int64_t size;
  struct {
    int64_t size;
    int64_t count;
  } pool_node;
  Allocator parent_allocator;
  Allocator backup_allocator;
};

Allocator create_allocator(const AllocatorCreateInfo *info);

void destry_allocator(Allocator allocator);

void *alloc(Allocator allocator, int64_t size);
void dealloc(Allocator allocator, void *ptr);

template <typename T> static constexpr int64_t size_of() {
  return int64_t{sizeof(T)};
}

template <typename T> T *make_new(Allocator allocator) {
  return static_cast<T *>(alloc(allocator, size_of<T>()));
}
template <typename T> T *make_arr(Allocator allocator, int64_t count) {
  return static_cast<T *>(alloc(allocator, size_of<T>() * count));
}

template <typename T> void destroy(Allocator allocator, const T *ptr) {
  dealloc(allocator, ptr);
}
}

#endif // MEMZ2_H
