#include "memz2.h"

#include <assert.h>
#include <cstdint>
#include <malloc.h>
#include <string.h>

static constexpr int64_t default_alignment{sizeof(void *)};
static constexpr int64_t page_size{4096};

template <typename T> static inline T align_block(T alignment, T size) {
  return (size + (alignment - 1)) & ~(alignment - 1);
}

template <typename T> static inline T max(T x, T y) {
  return x ^ ((x ^ y) & -(x < y));
}

template <typename T> static inline T min(T x, T y) {
  return y ^ ((x ^ y) & -(x < y));
}

template <typename T> static constexpr size_t to_size_t(T val) {
  return static_cast<size_t>(val);
}

struct internal_blk {
  char *ptr;
  int64_t size;
};

namespace memz2 {
struct Allocator_T {
  AllocatorType type;
  Allocator parent;
  char *buffer;
  int64_t size;

  char *internal_buffer;
  int64_t internal_size;
};

struct StackAllocator : memz2::Allocator_T {
  char *cursor;
};

struct PoolAllocator : memz2::Allocator_T {
  struct node_t {
    node_t *next;
  } * root;
  int64_t node_size;
};

struct BlockAllocator : memz2::Allocator_T {
  struct node_t {
    char *ptr;
    int64_t size;
  } * blocks;
  int64_t block_count;
};

static inline bool owns(Allocator allocator, void *ptr) {
  return allocator->buffer <= ptr && allocator->buffer + allocator->size > ptr;
}

char *allocate_malloc(int64_t size) {
  return static_cast<char *>(malloc(to_size_t(size)));
}

void free_malloc(char *ptr) { free(ptr); }

internal_blk allocate_stack(StackAllocator *allocator, int64_t size) {
  char *res = nullptr;
  if (allocator->cursor + size < allocator->buffer + allocator->size) {
    res = allocator->cursor;
    allocator->cursor += size;
  }
  return {res, size};
}

void free_stack(StackAllocator *allocator, internal_blk mem) {
  if (allocator->cursor - mem.size == mem.ptr) {
    allocator->cursor -= mem.size;
  }
}

internal_blk allocate_pool(PoolAllocator *allocator, int64_t size) {

  char *res = nullptr;
  if (allocator->node_size >= size && allocator->root) {
    PoolAllocator::node_t *currect = allocator->root;
    allocator->root = allocator->root->next;
    res = reinterpret_cast<char *>(currect);
  }
  return {res, allocator->node_size};
}

void free_pool(PoolAllocator *allocator, internal_blk mem) {
  assert(allocator->node_size == mem.size);
  PoolAllocator::node_t *node =
      reinterpret_cast<PoolAllocator::node_t *>(mem.ptr);
  node->next = allocator->root;
  allocator->root = node;
}

internal_blk allocate_block(BlockAllocator *allocator, int64_t size) {
  char *res = nullptr;
  BlockAllocator::node_t *root = allocator->blocks;
  const int64_t count = allocator->block_count;

  int64_t idx;
  if (count == 1) {
    idx = 0;
  } else {
    for (idx = 0; idx < count; ++idx) {
      if (root[idx].size >= size) {
        break;
      }
    }
  }

  assert(idx < count);
  BlockAllocator::node_t *blk = &root[idx];

  int64_t remainder = blk->size - size;
  res = blk->ptr;

  if (remainder < default_alignment) {
    size = blk->size;
    int64_t next_idx = idx + 1;
    if (next_idx < count) {
      memmove(blk, &root[next_idx], sizeof(BlockAllocator::node_t) *
                                        static_cast<size_t>(count - idx));
    }
    --allocator->block_count;
  } else {
    blk->size = remainder;
    blk->ptr = blk->ptr + size;
  }

  return {res, size};
}

void free_block(BlockAllocator *allocator, internal_blk mem) {

  BlockAllocator::node_t *root = allocator->blocks;
  const int64_t count = allocator->block_count;

  int64_t idx;
  if (count == 1) {
    idx = 0;
  } else {
    constexpr int64_t stride{8};
    for (idx = stride; idx < count; idx += stride) {
      if (root[idx].ptr > mem.ptr) {
        break;
      }
    }
    const int64_t free_idx_end = min(idx, count);
    for (int64_t i = idx - stride; i < free_idx_end; i++) {
      if (root[i].ptr > mem.ptr) {
        idx = i;
        break;
      }
    }
  }

  assert(idx < count);

  BlockAllocator::node_t *blk = &root[idx];
  BlockAllocator::node_t *prev_blk = idx > 0 ? &root[idx - 1] : nullptr;

  int right_merge = (mem.ptr + mem.size) == blk->ptr;
  int left_merge = prev_blk && (prev_blk->ptr + prev_blk->size) == mem.ptr;

  if (right_merge && left_merge) {
    prev_blk->size += mem.size + blk->size;

    int64_t next_idx = idx + 1;
    if (next_idx < count) {
      memmove(blk, &root[next_idx], sizeof(BlockAllocator::node_t) *
                                        static_cast<size_t>(count - idx));
    }
    --allocator->block_count;
  } else if (right_merge) {
    blk->ptr = mem.ptr;
    blk->size += mem.size;
  } else if (left_merge) {
    prev_blk->size += mem.size;
  } else {
    memmove(&root[idx + 1], blk,
            sizeof(BlockAllocator::node_t) * static_cast<size_t>(count - idx));
    blk->ptr = mem.ptr;
    blk->size = mem.size;
    ++allocator->block_count;
  }
}

static inline internal_blk alloc_internal(Allocator allocator, int64_t size) {

  size = align_block(default_alignment, size);

  internal_blk blk;
  if (allocator) {
    switch (allocator->type) {
    case AllocatorType::STACK: {
      blk = allocate_stack(static_cast<StackAllocator *>(allocator), size);
      break;
    }
    case AllocatorType::POOL: {
      blk = allocate_pool(static_cast<PoolAllocator *>(allocator), size);
      break;
    }
    case AllocatorType::BLOCK: {
      blk = allocate_block(static_cast<BlockAllocator *>(allocator), size);
      break;
    }
    }
  } else {
    blk = {allocate_malloc(size), size};
  }

  return blk;
}

static inline void free_internal(Allocator allocator, internal_blk mem) {
  if (allocator) {
    switch (allocator->type) {
    case AllocatorType::STACK: {
      free_stack(static_cast<StackAllocator *>(allocator), mem);
      break;
    }
    case AllocatorType::POOL: {
      free_pool(static_cast<PoolAllocator *>(allocator), mem);
      break;
    }
    case AllocatorType::BLOCK: {
      free_block(static_cast<BlockAllocator *>(allocator), mem);
      break;
    }
    }
  } else {
    free_malloc(mem.ptr);
  }
}

Allocator create_allocator(const AllocatorCreateInfo *info) {

  switch (info->type) {
  case AllocatorType::STACK: {

    const int64_t alloc_size = size_of<StackAllocator>();
    const int64_t size = align_block(page_size, info->size + alloc_size);

    internal_blk mem = alloc_internal(info->parent_allocator, size);
    assert(mem.ptr != nullptr);
    StackAllocator *alloc_tmp = reinterpret_cast<StackAllocator *>(mem.ptr);

    alloc_tmp->type = AllocatorType::STACK;
    alloc_tmp->parent = info->parent_allocator;
    alloc_tmp->buffer = mem.ptr + alloc_size;
    alloc_tmp->size = mem.size - alloc_size;
    alloc_tmp->internal_buffer = mem.ptr;
    alloc_tmp->internal_size = mem.size;

    alloc_tmp->cursor = alloc_tmp->buffer;

    return alloc_tmp;
  }

  case AllocatorType::POOL: {

    const int64_t node_size =
        align_block(default_alignment, info->pool_node.size);
    const int64_t alloc_size = size_of<PoolAllocator>();
    const int64_t size = align_block(
        page_size, (node_size * info->pool_node.count) + alloc_size);

    internal_blk mem = alloc_internal(info->parent_allocator, size);
    assert(mem.ptr != nullptr);
    PoolAllocator *alloc_tmp = reinterpret_cast<PoolAllocator *>(mem.ptr);

    alloc_tmp->type = AllocatorType::POOL;
    alloc_tmp->parent = info->parent_allocator;
    alloc_tmp->buffer = mem.ptr + alloc_size;
    alloc_tmp->size = mem.size - alloc_size;
    alloc_tmp->internal_buffer = mem.ptr;
    alloc_tmp->internal_size = mem.size;

    PoolAllocator::node_t *current = nullptr;
    for (int64_t i = info->pool_node.count - 1; i >= 0; --i) {
      PoolAllocator::node_t *tmp = reinterpret_cast<PoolAllocator::node_t *>(
          alloc_tmp->buffer + (node_size * i));
      tmp->next = current;
      current = tmp;
    }
    alloc_tmp->root = current;
    return alloc_tmp;
  }

  case AllocatorType::BLOCK: {

    const int64_t block_count = align_block(page_size, info->size) / page_size;
    const int64_t alloc_size = size_of<BlockAllocator>();
    const int64_t block_size = block_count * size_of<BlockAllocator::node_t>();
    const int64_t size =
        align_block(page_size, info->size + block_size + alloc_size);

    internal_blk mem = alloc_internal(info->parent_allocator, size);
    assert(mem.ptr != nullptr);
    BlockAllocator *alloc_tmp = reinterpret_cast<BlockAllocator *>(mem.ptr);

    alloc_tmp->type = AllocatorType::BLOCK;
    alloc_tmp->parent = info->parent_allocator;
    alloc_tmp->buffer = mem.ptr + alloc_size + block_size;
    alloc_tmp->size = mem.size - alloc_size - block_size;
    alloc_tmp->internal_buffer = mem.ptr;
    alloc_tmp->internal_size = mem.size;

    BlockAllocator::node_t *blocks =
        reinterpret_cast<BlockAllocator::node_t *>(mem.ptr + alloc_size);
    memset(blocks, 0, to_size_t(block_size));
    blocks[0].ptr = alloc_tmp->buffer;
    blocks[0].size = alloc_tmp->size;

    alloc_tmp->blocks = blocks;
    alloc_tmp->block_count = 1;

    return alloc_tmp;
  }
  }

  return nullptr;
}

void destry_allocator(Allocator allocator) {
  free_internal(allocator->parent,
                {allocator->internal_buffer, allocator->internal_size});
}

void *alloc(Allocator allocator, int64_t size) {
  internal_blk mem = alloc_internal(allocator, size + size_of<int64_t>());
  *reinterpret_cast<int64_t *>(mem.ptr) = mem.size;
  return mem.ptr + size_of<int64_t>();
}

void dealloc(Allocator allocator, void *ptr) {
  assert(memz2::owns(allocator, ptr));
  char *p = static_cast<char *>(ptr) - size_of<int64_t>();
  int64_t s = *reinterpret_cast<int64_t *>(p);
  free_internal(allocator, {p, s});
}
}
