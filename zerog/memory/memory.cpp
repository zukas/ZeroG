#include "memory.h"

#include "common/bitop.h"
#include "common/math.h"

#include <cassert>
#include <cstring>

constexpr size_t align_block(size_t alignment, size_t size) {
  return (size + (alignment - 1)) & ~(alignment - 1);
}

enum allocator_type : size_t { NONE, STACK, FREE_LIST, POOL, BITMAPED_BLOCK };

struct allocator {
  allocator_type type;
  allocator *parent;
  uint8_t *data;
  size_t size;
};

struct stack_allocator : allocator {
  uint8_t *cursor;
};

struct free_list_allocator : allocator {
  struct node_t {
    node_t *next;
  };
  size_t min_size;
  size_t max_size;
  node_t *root;
};

struct pool_allocator : allocator {
  struct node_t {
    node_t *next;
  };
  node_t *root;
  size_t node_size;
};

struct bitmapped_block_allocator : allocator {
  size_t block_size;
  uint64_t used_mask;
};

static constexpr size_t max_allocator_size_aligned{align_block(
    16,
    max(sizeof(stack_allocator),
        max(sizeof(free_list_allocator),
            max(sizeof(pool_allocator), sizeof(bitmapped_block_allocator)))))};

static constexpr size_t allocator_alignment{64};

static blk _alloc(stack_allocator *allocator, size_t size) {
  blk res{};
  constexpr size_t alignment{16};
  size_t asize = align_block(alignment, size);

  if (allocator->cursor + asize <= &allocator->data[allocator->size]) {
    res = {allocator->cursor, asize};
    allocator->cursor += asize;
  }
  return res;
}

static void _free(stack_allocator *allocator, blk data) {
  assert(data.ptr >= allocator->data &&
         data.ptr < &allocator->data[allocator->size] &&
         "Current stack allocator does not own a given block");
  if (allocator->cursor - data.size == data.ptr) {
    allocator->cursor -= data.size;
  }
}

static blk _alloc(free_list_allocator *allocator, size_t size) {
  blk res{};
  constexpr size_t alignment{16};
  size_t asize = align_block(alignment, size);

  if (allocator->max_size >= asize && allocator->min_size < asize &&
      allocator->root) {
    free_list_allocator::node_t *node = allocator->root;
    allocator->root = node->next;
    res = {node, asize};
  } else {
    res = allocate(allocator->parent, size);
  }
  return res;
}

static void _free(free_list_allocator *allocator, blk data) {
  if (allocator->max_size >= data.size && allocator->min_size < data.size) {
    auto node = static_cast<free_list_allocator::node_t *>(data.ptr);
    node->next = allocator->root;
    allocator->root = node;
  } else {
    deallocate(allocator->parent, data);
  }
}

static blk _alloc(pool_allocator *allocator, size_t size) {
  blk res{};
  constexpr size_t alignment{16};
  size_t asize = align_block(alignment, size);

  if (allocator->node_size >= asize && allocator->root) {
    auto tmp = allocator->root;
    allocator->root = tmp->next;
    res = {tmp, allocator->node_size};
  }
  return res;
}

static void _free(pool_allocator *allocator, blk data) {
  assert(data.ptr >= allocator->data &&
         data.ptr < &allocator->data[allocator->size] &&
         "Current pool allocator does not own a given block");

  if (allocator->node_size == data.size) {
    auto node = static_cast<pool_allocator::node_t *>(data.ptr);
    node->next = allocator->root;
    allocator->root = node;
  }
}

static int32_t bitmapped_block_multiplier(size_t block_size, size_t size) {

  if (size == 0)
    return 0;
  const size_t block_count_1 = block_size;
  const size_t block_count_2 = block_size * 2;
  const size_t block_count_3 = block_size * 3;
  const size_t block_count_4 = block_size * 4;
  if (size == block_count_1)
    return 1;
  else if (size == block_count_2)
    return 2;
  else if (size == block_count_3)
    return 3;
  else if (size == block_count_4)
    return 4;

  int32_t markers[]{32, 16, 8, 4, 2, 1};
  int32_t current = 0;
  for (int32_t i = 0; i < 6; ++i) {
    const int32_t pos = current + markers[i];
    const size_t asize = block_size * static_cast<size_t>(pos);
    if (asize == size) {
      return pos;
    } else if (asize < size) {
      current = pos;
    }
  }
  return 64;
}

static blk _alloc(bitmapped_block_allocator *allocator, size_t size) {
  blk res{};
  constexpr size_t alignment{64};
  size_t asize = align_block(alignment, size);

  const size_t block_size = allocator->block_size;
  const size_t blocks = align_block(allocator->block_size, asize);
  const int32_t count =
      bitmapped_block_multiplier(block_size, blocks); // blocks / block_size;

  const int32_t idx = find_mask_bits(allocator->used_mask, count);
  if (idx >= 0) {
    allocator->used_mask = set_mask(allocator->used_mask, idx, count);
    res = {&allocator->data[block_size * static_cast<size_t>(idx)], blocks};
  }
  return res;
}

static void _free(bitmapped_block_allocator *allocator, blk data) {
  assert(data.ptr >= allocator->data &&
         data.ptr < &allocator->data[allocator->size] &&
         "Current bitmaped allocator does not own a given block");

  size_t len =
      static_cast<size_t>(static_cast<uint8_t *>(data.ptr) - allocator->data);
  assert(len >= 0 &&
         "Allocator failed to determine the order of data correctly. Failed");

  const size_t block_size = allocator->block_size;
  const size_t blocks = align_block(allocator->block_size, data.size);
  const int32_t count =
      bitmapped_block_multiplier(block_size, blocks); // blocks / block_size;
  const int32_t idx =
      bitmapped_block_multiplier(block_size, len); // len / block_size;

  allocator->used_mask = unset_mask(allocator->used_mask, idx, count);
}

blk allocate(allocator *allocator, size_t size) {
  assert(allocator && "Allocator is null");
  switch (allocator->type) {
  case NONE: {
    assert(0 && "Allocator is not valid");
    return {nullptr, 0};
  }
  case STACK: {
    return _alloc(static_cast<stack_allocator *>(allocator), size);
  }
  case FREE_LIST: {
    return _alloc(static_cast<free_list_allocator *>(allocator), size);
  }
  case POOL: {
    return _alloc(static_cast<pool_allocator *>(allocator), size);
  }
  case BITMAPED_BLOCK: {
    return _alloc(static_cast<bitmapped_block_allocator *>(allocator), size);
  }
  }
  assert(0 && "No allocation strategy matched");
  return {nullptr, 0};
}

void deallocate(allocator *allocator, blk block) {
  assert(allocator && "Allocator is null");
  switch (allocator->type) {
  case NONE: {
    assert(0 && "Allocator is not valid");
    break;
  }
  case STACK: {
    _free(static_cast<stack_allocator *>(allocator), block);
    break;
  }
  case FREE_LIST: {
    _free(static_cast<free_list_allocator *>(allocator), block);
    break;
  }
  case POOL: {
    _free(static_cast<pool_allocator *>(allocator), block);
    break;
  }
  case BITMAPED_BLOCK: {
    _free(static_cast<bitmapped_block_allocator *>(allocator), block);
    break;
  }
  }
}

allocator *create_stack_allocator(size_t size) {

  size_t asize = align_block(allocator_alignment, size);
  size_t full_size = max_allocator_size_aligned + asize;

  uint8_t *raw = static_cast<uint8_t *>(malloc(full_size));

  assert(raw && "Failed to allocated data");

  uint8_t *data = raw + max_allocator_size_aligned;

  stack_allocator *alloc = reinterpret_cast<stack_allocator *>(raw);
  alloc->type = STACK;
  alloc->parent = nullptr;
  alloc->data = data;
  alloc->size = asize;
  alloc->cursor = data;
  return alloc;
}

allocator *create_stack_allocator(size_t size, allocator *parent) {
  assert(parent && "Requires a valid parent allocator");

  size_t asize = align_block(allocator_alignment, size);
  size_t full_size = max_allocator_size_aligned + asize;

  blk b = allocate(parent, full_size);

  assert(b.ptr && "Failed to allocated data");

  uint8_t *raw = static_cast<uint8_t *>(b.ptr);
  uint8_t *data = raw + max_allocator_size_aligned;

  stack_allocator *alloc = reinterpret_cast<stack_allocator *>(raw);
  alloc->type = STACK;
  alloc->parent = parent;
  alloc->data = data;
  alloc->size = b.size - max_allocator_size_aligned;
  alloc->cursor = data;
  return alloc;
}

allocator *create_free_list_allocator(size_t min_block, size_t max_block,
                                      allocator *parent) {
  assert(parent && "Requires a valid parent allocator");

  blk b = allocate(parent, max_allocator_size_aligned);

  assert(b.ptr && "Failed to allocated data");

  free_list_allocator *alloc = static_cast<free_list_allocator *>(b.ptr);
  alloc->type = FREE_LIST;
  alloc->parent = parent;
  alloc->data = nullptr;
  alloc->size = b.size - max_allocator_size_aligned;
  alloc->min_size = min_block;
  alloc->max_size = max_block;
  alloc->root = nullptr;
  return alloc;
}

allocator *create_pool_allocator(size_t block_size, size_t block_count) {

  constexpr size_t alignment{64};
  size_t asize = align_block(alignment, block_size);
  size_t full_size = max_allocator_size_aligned + (asize * block_count);

  uint8_t *raw = static_cast<uint8_t *>(malloc(full_size));

  assert(raw && "Failed to allocated data");

  uint8_t *data = raw + max_allocator_size_aligned;

  pool_allocator::node_t *nodes =
      reinterpret_cast<pool_allocator::node_t *>(data);
  pool_allocator *allocator = reinterpret_cast<pool_allocator *>(raw);

  allocator->type = POOL;
  allocator->parent = nullptr;
  allocator->root = &nodes[0];
  allocator->node_size = asize;
  allocator->data = data;
  allocator->size = asize * block_count;

  for (size_t i = 1; i < block_count; i++) {
    nodes[i - 1].next = &nodes[i];
  }

  return allocator;
}

allocator *create_pool_allocator(size_t block_size, size_t block_count,
                                 allocator *parent) {
  constexpr size_t alignment{64};
  size_t asize = align_block(alignment, block_size);
  size_t full_size = max_allocator_size_aligned + (asize * block_count);

  blk b = allocate(parent, full_size);

  assert(b.ptr && "Failed to allocated data");

  uint8_t *raw = static_cast<uint8_t *>(b.ptr);
  uint8_t *data = raw + max_allocator_size_aligned;

  pool_allocator::node_t *nodes =
      reinterpret_cast<pool_allocator::node_t *>(data);
  pool_allocator *allocator = reinterpret_cast<pool_allocator *>(raw);

  allocator->type = POOL;
  allocator->parent = parent;
  allocator->data = data;
  allocator->size = b.size - max_allocator_size_aligned;
  allocator->root = &nodes[0];
  allocator->node_size = asize;

  for (size_t i = 1; i < block_count; i++) {
    nodes[i - 1].next = &nodes[i];
  }
  return allocator;
}

allocator *create_bitmapped_allocator(size_t block_size) {
  constexpr size_t alignment{sizeof64};

  size_t asize = align_block(alignment, block_size);
  size_t full_size = max_allocator_size_aligned + (asize * sizeof64);

  uint8_t *raw = static_cast<uint8_t *>(malloc(full_size));

  assert(raw && "Failed to allocated data");
  uint8_t *data = raw + max_allocator_size_aligned;
  bitmapped_block_allocator *alloc =
      reinterpret_cast<bitmapped_block_allocator *>(raw);

  alloc->type = BITMAPED_BLOCK;
  alloc->parent = nullptr;
  alloc->data = data;
  alloc->size = full_size - max_allocator_size_aligned;
  alloc->block_size = asize;
  alloc->used_mask = 0;
  return alloc;
}

allocator *create_bitmapped_allocator(size_t block_size, allocator *parent) {
  constexpr size_t alignment{64};
  size_t asize = align_block(alignment, block_size);
  size_t full_size = max_allocator_size_aligned + (asize * sizeof64);

  blk b = allocate(parent, full_size);

  assert(b.ptr && "Failed to allocated data");

  uint8_t *raw = static_cast<uint8_t *>(b.ptr);
  uint8_t *data = raw + max_allocator_size_aligned;
  bitmapped_block_allocator *alloc =
      reinterpret_cast<bitmapped_block_allocator *>(raw);

  alloc->type = BITMAPED_BLOCK;
  alloc->parent = parent;
  alloc->data = data;
  alloc->size = b.size - max_allocator_size_aligned;
  alloc->block_size = asize;
  alloc->used_mask = 0;
  return alloc;
}

void reset_allocator(allocator *allocator) {
  assert(allocator && "Allocator is null");
  switch (allocator->type) {
  case NONE: {
    assert(0 && "Allocator is not valid");
    break;
  }
  case STACK: {
    stack_allocator *alloc = static_cast<stack_allocator *>(allocator);
    alloc->cursor = alloc->data;
    break;
  }
  case FREE_LIST: {
    free_list_allocator *alloc = static_cast<free_list_allocator *>(allocator);
    alloc->root = nullptr;
    reset_allocator(alloc->parent);
    break;
  }
  case POOL: {
    pool_allocator *alloc = static_cast<pool_allocator *>(allocator);
    pool_allocator::node_t *nodes =
        reinterpret_cast<pool_allocator::node_t *>(alloc->data);
    size_t block_count = alloc->size / alloc->node_size;
    for (size_t i = 1; i < block_count; i++) {
      nodes[i - 1].next = &nodes[i];
    }
    break;
  }
  case BITMAPED_BLOCK: {
    bitmapped_block_allocator *alloc =
        static_cast<bitmapped_block_allocator *>(allocator);
    alloc->used_mask = 0;
    break;
  }
  }
}

void destroy_allocator(allocator *allocator) {
  assert(allocator && "Allocator is null");
  if (allocator->parent) {
    deallocate(allocator->parent,
               {allocator, allocator->size + max_allocator_size_aligned});
  } else {
    free(allocator);
  }
}
