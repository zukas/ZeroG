#ifndef MEMORY_H
#define MEMORY_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>

struct blk {
  void *ptr;
  size_t size;
};

constexpr size_t Kb{1024};
constexpr size_t Mb{1024 * Kb};
constexpr size_t Gb{1024 * Mb};

struct allocator;

blk allocate(allocator *allocator, size_t size);
void deallocate(allocator *allocator, blk block);

allocator *create_stack_allocator(size_t size);
allocator *create_stack_allocator(size_t size, allocator *parent);

allocator *create_free_list_allocator(size_t min_block, size_t max_block,
                                      allocator *parent);

allocator *create_pool_allocator(size_t block_size, size_t block_count);
allocator *create_pool_allocator(size_t block_size, size_t block_count,
                                 allocator *parent);

allocator *create_bitmapped_allocator(size_t block_size);
allocator *create_bitmapped_allocator(size_t block_size, allocator *parent);

void reset_allocator(allocator *allocator);
void destroy_allocator(allocator *allocator);

#define ARRAY_DEFINITION(name, type)                                           \
  struct name {                                                                \
    typedef type __T;                                                          \
    typedef __T *iterator;                                                     \
    typedef const __T *const_iterator;                                         \
    __T *data;                                                                 \
    size_t count;                                                              \
    size_t size;                                                               \
    iterator begin();                                                          \
    const_iterator begin() const;                                              \
    iterator end();                                                            \
    const_iterator end() const;                                                \
  };                                                                           \
  name create_##name(allocator *alloc, size_t size);                           \
  void destroy_##name(allocator *alloc, name array)

#define ARRAY_IMPLEMENTATION(name)                                             \
  name::iterator name::begin() { return &data[0]; }                            \
  name::const_iterator name::begin() const { return &data[0]; }                \
  name::iterator name::end() { return &data[count]; }                          \
  name::const_iterator name::end() const { return &data[count]; }              \
  name create_##name(allocator *alloc, size_t size) {                          \
    name t;                                                                    \
    blk b = allocate(alloc, sizeof(name::__T) * size);                         \
    t.data = static_cast<name::__T *>(b.ptr);                                  \
    t.count = size;                                                            \
    t.size = b.size;                                                           \
    return t;                                                                  \
  }                                                                            \
  void destroy_##name(allocator *alloc, name array) {                          \
    deallocate(alloc, {array.data, array.size});                               \
  }

#define PTR_DEFINITION(type)                                                   \
  struct type##_ptr {                                                          \
    type *ptr;                                                                 \
    size_t size;                                                               \
  };                                                                           \
  type##_ptr create_##type(allocator *alloc);                                  \
  void destroy_##type(allocator *alloc, type##_ptr ptr)

#define PTR_IMPLEMENTATION(type)                                               \
  type##_ptr create_##type(allocator *alloc) {                                 \
    blk b = allocate(alloc, sizeof(type));                                     \
    return {static_cast<type *>(b.ptr), b.size};                               \
  }                                                                            \
  void destroy_##type(allocator *alloc, type##_ptr ptr) {                      \
    deallocate(alloc, {ptr.ptr, ptr.size});                                    \
  }

#endif // MEMORY_H
