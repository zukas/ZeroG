#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include "memory/memory.h"

using namespace testing;

TEST(allocator, stack_allocator_creation) {
  allocator *alloc = create_stack_allocator(Gb);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 512);
  EXPECT_EQ(b.size, Mb * 512);
  EXPECT_NE(b.ptr, nullptr);
  destroy_allocator(alloc);
}

TEST(allocator, stack_allocator_use_all) {
  allocator *alloc = create_stack_allocator(Gb);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 512);
  EXPECT_EQ(b.size, Mb * 512);
  EXPECT_NE(b.ptr, nullptr);
  blk b2 = allocate(alloc, Mb * 512);
  EXPECT_EQ(b2.size, Mb * 512);
  EXPECT_NE(b2.ptr, nullptr);
  EXPECT_NE(b.ptr, b2.ptr);

  blk b3 = allocate(alloc, Mb);
  EXPECT_EQ(b3.size, 0);
  EXPECT_EQ(b3.ptr, nullptr);

  destroy_allocator(alloc);
}

TEST(allocator, stack_allocator_alloc) {
  allocator *alloc = create_stack_allocator(Gb);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 512);
  EXPECT_EQ(b.size, Mb * 512);
  EXPECT_NE(b.ptr, nullptr);
  blk b2 = allocate(alloc, Mb * 512);
  EXPECT_EQ(b2.size, Mb * 512);
  EXPECT_NE(b2.ptr, nullptr);
  EXPECT_NE(b.ptr, b2.ptr);

  deallocate(alloc, b2);
  deallocate(alloc, b);

  blk b3 = allocate(alloc, Mb);
  EXPECT_EQ(b3.size, Mb);
  EXPECT_NE(b3.ptr, nullptr);

  destroy_allocator(alloc);
}

TEST(allocator, stack_with_free_list_creation) {
  allocator *s_alloc = create_stack_allocator(Gb);
  EXPECT_NE(s_alloc, nullptr);

  allocator *l1_alloc =
      create_free_list_allocator(sizeof(void *), Mb * 64, s_alloc);
  EXPECT_NE(l1_alloc, nullptr);

  blk b = allocate(l1_alloc, Mb);
  EXPECT_EQ(b.size, Mb);
  EXPECT_NE(b.ptr, nullptr);

  destroy_allocator(l1_alloc);
  destroy_allocator(s_alloc);
}

TEST(allocator, stack_with_free_list_allocate) {
  allocator *s_alloc = create_stack_allocator(Gb);
  EXPECT_NE(s_alloc, nullptr);

  allocator *l1_alloc =
      create_free_list_allocator(sizeof(void *), Mb * 64, s_alloc);
  EXPECT_NE(l1_alloc, nullptr);

  blk b1 = allocate(l1_alloc, Mb);
  EXPECT_EQ(b1.size, Mb);
  EXPECT_NE(b1.ptr, nullptr);

  blk b2 = allocate(l1_alloc, Mb);
  EXPECT_EQ(b2.size, Mb);
  EXPECT_NE(b2.ptr, nullptr);

  blk b3 = allocate(l1_alloc, Mb);
  EXPECT_EQ(b3.size, Mb);
  EXPECT_NE(b3.ptr, nullptr);

  blk b4 = allocate(l1_alloc, Mb);
  EXPECT_EQ(b4.size, Mb);
  EXPECT_NE(b4.ptr, nullptr);

  deallocate(l1_alloc, b1);
  deallocate(l1_alloc, b3);

  blk b5 = allocate(l1_alloc, Mb * 32);
  EXPECT_EQ(b5.size, Mb * 32);
  EXPECT_NE(b5.ptr, nullptr);

  blk b6 = allocate(l1_alloc, Mb * 32);
  EXPECT_EQ(b6.size, Mb * 32);
  EXPECT_NE(b6.ptr, nullptr);

  destroy_allocator(l1_alloc);
  destroy_allocator(s_alloc);
}

TEST(allocator, stack_with_free_list_level_5) {
  allocator *s_alloc = create_stack_allocator(Gb);
  EXPECT_NE(s_alloc, nullptr);

  allocator *l1_alloc = create_free_list_allocator(Mb * 32, Mb * 64, s_alloc);
  EXPECT_NE(l1_alloc, nullptr);

  allocator *l2_alloc = create_free_list_allocator(Mb * 16, Mb * 32, l1_alloc);
  EXPECT_NE(l2_alloc, nullptr);

  allocator *l3_alloc = create_free_list_allocator(Mb * 8, Mb * 16, l2_alloc);
  EXPECT_NE(l3_alloc, nullptr);

  allocator *l4_alloc = create_free_list_allocator(Mb, Mb * 8, l3_alloc);
  EXPECT_NE(l4_alloc, nullptr);

  allocator *l5_alloc =
      create_free_list_allocator(sizeof(void *), Mb, l4_alloc);
  EXPECT_NE(l5_alloc, nullptr);

  blk b1 = allocate(l5_alloc, Kb * 512);
  EXPECT_EQ(b1.size, Kb * 512);
  EXPECT_NE(b1.ptr, nullptr);

  blk b2 = allocate(l5_alloc, Mb * 4);
  EXPECT_EQ(b2.size, Mb * 4);
  EXPECT_NE(b2.ptr, nullptr);

  blk b3 = allocate(l5_alloc, Mb * 12);
  EXPECT_EQ(b3.size, Mb * 12);
  EXPECT_NE(b3.ptr, nullptr);

  blk b4 = allocate(l5_alloc, Mb * 28);
  EXPECT_EQ(b4.size, Mb * 28);
  EXPECT_NE(b4.ptr, nullptr);

  blk b5 = allocate(l5_alloc, Mb * 56);
  EXPECT_EQ(b5.size, Mb * 56);
  EXPECT_NE(b5.ptr, nullptr);

  deallocate(l5_alloc, b1);
  deallocate(l5_alloc, b2);
  deallocate(l5_alloc, b3);
  deallocate(l5_alloc, b4);
  deallocate(l5_alloc, b5);

  blk b6 = allocate(l5_alloc, Kb * 512);
  EXPECT_EQ(b6.size, Kb * 512);
  EXPECT_EQ(b6.ptr, b1.ptr);

  blk b7 = allocate(l5_alloc, Mb * 4);
  EXPECT_EQ(b7.size, Mb * 4);
  EXPECT_EQ(b7.ptr, b2.ptr);

  blk b8 = allocate(l5_alloc, Mb * 12);
  EXPECT_EQ(b8.size, Mb * 12);
  EXPECT_EQ(b8.ptr, b3.ptr);

  blk b9 = allocate(l5_alloc, Mb * 28);
  EXPECT_EQ(b9.size, Mb * 28);
  EXPECT_EQ(b9.ptr, b4.ptr);

  blk b10 = allocate(l5_alloc, Mb * 56);
  EXPECT_EQ(b10.size, Mb * 56);
  EXPECT_EQ(b10.ptr, b5.ptr);

  destroy_allocator(s_alloc);
}

TEST(allocator, pool_allocator_creation) {
  allocator *alloc = create_pool_allocator(Mb, 1024);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Kb * 512);
  EXPECT_EQ(b.size, Mb);
  EXPECT_NE(b.ptr, nullptr);
  destroy_allocator(alloc);
}

TEST(allocator, pool_allocator_full_allocate) {
  allocator *alloc = create_pool_allocator(Mb, 1024);
  EXPECT_NE(alloc, nullptr);

  blk blks[1024];
  for (int i = 0; i < 1024; i++) {
    blks[i] = allocate(alloc, Kb * 512);
    EXPECT_EQ(blks[i].size, Mb);
    EXPECT_NE(blks[i].ptr, nullptr);
  }
  for (int i = 0; i < 1024; i++) {
    deallocate(alloc, blks[i]);
  }
  destroy_allocator(alloc);
}

TEST(allocator, pool_allocator_full_allocate_repeat) {
  allocator *alloc = create_pool_allocator(Mb, 1024);
  EXPECT_NE(alloc, nullptr);

  blk blks[1024];
  for (int i = 0; i < 1024; i++) {
    for (int i = 0; i < 1024; i++) {
      blks[i] = allocate(alloc, Kb * 512);
      EXPECT_EQ(blks[i].size, Mb);
      EXPECT_NE(blks[i].ptr, nullptr);
    }
    for (int i = 0; i < 1024; i++) {
      deallocate(alloc, blks[i]);
    }
  }
  destroy_allocator(alloc);
}

TEST(allocator, pool_allocator_all_to_null) {
  allocator *alloc = create_pool_allocator(Mb, 1024);
  EXPECT_NE(alloc, nullptr);

  blk blks[1024];
  for (int i = 0; i < 1024; i++) {
    blks[i] = allocate(alloc, Kb * 512);
    EXPECT_EQ(blks[i].size, Mb);
    EXPECT_NE(blks[i].ptr, nullptr);
  }
  blk b = allocate(alloc, Kb * 512);
  EXPECT_EQ(b.size, 0);
  EXPECT_EQ(b.ptr, nullptr);

  for (int i = 0; i < 1024; i++) {
    deallocate(alloc, blks[i]);
  }
  destroy_allocator(alloc);
}

TEST(allocator, bitmapped_allocator_create) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 55);
  EXPECT_EQ(b.size, Mb * 64);
  EXPECT_NE(b.ptr, nullptr);
  destroy_allocator(alloc);
}

TEST(allocator, bitmapped_allocator_group_allocate) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 77);
  EXPECT_EQ(b.size, Mb * 128);
  EXPECT_NE(b.ptr, nullptr);
  destroy_allocator(alloc);
}

TEST(allocator, bitmapped_allocator_allocate_all_in_one) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 64 * 64);
  EXPECT_EQ(b.size, Mb * 64 * 64);
  EXPECT_NE(b.ptr, nullptr);
  destroy_allocator(alloc);
}

TEST(allocator, bitmapped_allocator_allocate_all_in_parts) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  EXPECT_NE(alloc, nullptr);
  blk b = allocate(alloc, Mb * 64 * 16);
  EXPECT_EQ(b.size, Mb * 64 * 16);
  EXPECT_NE(b.ptr, nullptr);

  blk b2 = allocate(alloc, Mb * 64 * 16);
  EXPECT_EQ(b2.size, Mb * 64 * 16);
  EXPECT_NE(b2.ptr, nullptr);

  blk b3 = allocate(alloc, Mb * 64 * 8);
  EXPECT_EQ(b3.size, Mb * 64 * 8);
  EXPECT_NE(b3.ptr, nullptr);

  blk b4 = allocate(alloc, Mb * 64 * 8);
  EXPECT_EQ(b4.size, Mb * 64 * 8);
  EXPECT_NE(b4.ptr, nullptr);

  blk b5 = allocate(alloc, Mb * 64 * 8);
  EXPECT_EQ(b5.size, Mb * 64 * 8);
  EXPECT_NE(b5.ptr, nullptr);

  blk b6 = allocate(alloc, Mb * 64 * 8);
  EXPECT_EQ(b6.size, Mb * 64 * 8);
  EXPECT_NE(b6.ptr, nullptr);

  blk be = allocate(alloc, Mb * 64);
  EXPECT_EQ(be.size, 0);
  EXPECT_EQ(be.ptr, nullptr);

  deallocate(alloc, b5);

  blk b7 = allocate(alloc, Mb * 64 * 4);
  EXPECT_EQ(b7.size, Mb * 64 * 4);
  EXPECT_NE(b7.ptr, nullptr);

  blk b8 = allocate(alloc, Mb * 64 * 4);
  EXPECT_EQ(b8.size, Mb * 64 * 4);
  EXPECT_NE(b8.ptr, nullptr);

  destroy_allocator(alloc);
}
