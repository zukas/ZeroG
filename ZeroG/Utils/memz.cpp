#include "memz.h"

#include <assert.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MIN_ALIGNMENT sizeof(void *)

struct block_node_t {
  void *ptr;
  size_t size;
};

static struct {
  struct block_node_t *free_blk;
  int64_t free_count;

  //  int64_t total_dealloc;
  int64_t total_dealloc_it_max;

  void *buffer;
  size_t size;
} __global_heap;

template <typename T> static inline T align_block(T alignment, T size) {
  return (size + (alignment - 1)) & ~(alignment - 1);
}

static inline void *address_add(void *__restrict p, size_t adjust) {
  return static_cast<unsigned char *>(p) + adjust;
}

template <typename T> static inline T max(T x, T y) {
  return x ^ ((x ^ y) & -(x < y));
}

template <typename T> static inline T min(T x, T y) {
  return y ^ ((x ^ y) & -(x < y));
}

static inline bool owns(memz::Blk blk) {
  return __global_heap.buffer <= blk.ptr &&
         address_add(__global_heap.buffer, __global_heap.size) > blk.ptr;
}

void memz::init(size_t size) {

  size_t free_block_size = sizeof(struct block_node_t) * 1024 * 32;

  size = align_block(MIN_ALIGNMENT, size);

  void *buff = malloc(free_block_size + size);

  block_node_t *free_blocks = static_cast<block_node_t *>(buff);

  memset(free_blocks, 0, free_block_size);

  free_blocks[0].ptr = address_add(buff, free_block_size);
  free_blocks[0].size = size;

  __global_heap.free_blk = free_blocks;
  __global_heap.free_count = 1;

  __global_heap.buffer = buff;
  __global_heap.size = size;
}

void memz::deinit() {

  __global_heap.free_blk = nullptr;
  __global_heap.free_count = 0;

  free(__global_heap.buffer);

  __global_heap.buffer = nullptr;
  __global_heap.size = 0;
}

memz::Blk memz::alloc(size_t size) {

  void *result = nullptr;
  size = align_block(MIN_ALIGNMENT, size);

  struct block_node_t *root_free_blk = __global_heap.free_blk;
  const int64_t free_count = __global_heap.free_count;

  int64_t free_idx;
  if (free_count == 1) {
    free_idx = 0;
  } else {
    for (free_idx = 0; free_idx < free_count; free_idx++) {
      if (root_free_blk[free_idx].size >= size) {
        break;
      }
    }
  }

  assert(free_idx < free_count);
  struct block_node_t *free_blk = &root_free_blk[free_idx];

  size_t remainder = free_blk->size - size;
  result = free_blk->ptr;

  if (remainder < sizeof(void *)) {
    size = free_blk->size;
    int64_t free_next_idx = free_idx + 1;
    if (free_next_idx == free_count) {
      memset(free_blk, 0, sizeof(struct block_node_t));
    } else {
      memmove(free_blk, &root_free_blk[free_next_idx],
              sizeof(struct block_node_t) *
                  static_cast<size_t>(free_count - free_idx));
      memset(&root_free_blk[free_count - 1], 0, sizeof(struct block_node_t));
    }
    __global_heap.free_count--;
  } else {
    free_blk->size = remainder;
    free_blk->ptr = address_add(free_blk->ptr, size);
  }

  return {result, size};
}

memz::Blk memz::realloc(memz::Blk blk, size_t size) { return {}; }

memz::Blk memz::aligned(size_t alignment, size_t size) { return {}; }

void memz::dealloc(memz::Blk blk) {

  assert(owns(blk));

  const int64_t free_count = __global_heap.free_count;
  struct block_node_t *root_free_blk = __global_heap.free_blk;

  int64_t free_idx;
  if (free_count == 1) {
    free_idx = 0;
  } else {
    constexpr int64_t stride{8};
    for (free_idx = stride; free_idx < free_count; free_idx += stride) {
      if (root_free_blk[free_idx].ptr > blk.ptr) {
        break;
      }
    }
    const int64_t free_idx_end = min(free_idx, free_count);
    for (int64_t i = free_idx - stride; i < free_idx_end; i++) {
      if (root_free_blk[i].ptr > blk.ptr) {
        free_idx = i;
        break;
      }
    }
  }

  assert(free_idx < free_count);

  struct block_node_t *free_blk = &root_free_blk[free_idx];
  struct block_node_t *free__prev_blk =
      free_idx > 0 ? &root_free_blk[free_idx - 1] : nullptr;

  int right_merge = address_add(blk.ptr, blk.size) == free_blk->ptr;
  int left_merge =
      free__prev_blk &&
      address_add(free__prev_blk->ptr, free__prev_blk->size) == blk.ptr;

  if (right_merge && left_merge) {
    free__prev_blk->size += blk.size + free_blk->size;

    int64_t free_next_idx = free_idx + 1;
    if (free_next_idx == free_count) {
      memset(free_blk, 0, sizeof(struct block_node_t));
    } else {
      memmove(free_blk, &root_free_blk[free_next_idx],
              sizeof(struct block_node_t) *
                  static_cast<size_t>(free_count - free_idx));
      memset(&root_free_blk[free_count - 1], 0, sizeof(struct block_node_t));
    }
    __global_heap.free_count--;
  } else if (right_merge) {
    free_blk->ptr = blk.ptr;
    free_blk->size += blk.size;
  } else if (left_merge) {
    free__prev_blk->size += blk.size;
  } else {
    memmove(&root_free_blk[free_idx + 1], free_blk,
            sizeof(struct block_node_t) *
                static_cast<size_t>(free_count - free_idx));
    free_blk->ptr = blk.ptr;
    free_blk->size = blk.size;
    __global_heap.free_count++;
  }
}
