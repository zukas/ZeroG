
#include "benchmark/benchmark.h"
#include "memory/memory.h"

static void malloc_allocate_small(benchmark::State &state) {
  while (state.KeepRunning()) {
    auto data = malloc(75);
    benchmark::DoNotOptimize(data);
    free(data);
  }
}

static void malloc_allocate_mid(benchmark::State &state) {
  while (state.KeepRunning()) {
    auto data = malloc(Kb);
    benchmark::DoNotOptimize(data);
    free(data);
  }
}

static void malloc_allocate_large(benchmark::State &state) {
  while (state.KeepRunning()) {
    auto data = malloc(Mb);
    benchmark::DoNotOptimize(data);
    free(data);
  }
}

static void malloc_allocate_large_a6_d0(benchmark::State &state) {
  while (state.KeepRunning()) {
    auto b1 = malloc(Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);

    auto b2 = malloc(Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);

    auto b3 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);

    auto b4 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);

    auto b5 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);

    auto b6 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    state.PauseTiming();

    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b5);
    free(b6);
    state.ResumeTiming();
  }
}

static void malloc_allocate_large_a8_d1(benchmark::State &state) {
  while (state.KeepRunning()) {
    auto b1 = malloc(Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);

    auto b2 = malloc(Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);

    auto b3 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);

    auto b4 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);

    auto b5 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);

    auto b6 = malloc(Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    free(b5);

    auto b7 = malloc(Mb * 64 * 4);
    benchmark::DoNotOptimize(b7);

    auto b8 = malloc(Mb * 64 * 4);
    benchmark::DoNotOptimize(b8);

    state.PauseTiming();

    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b6);
    free(b7);
    free(b8);
    state.ResumeTiming();
  }
}

static void stack_allocator_cd(benchmark::State &state) {
  while (state.KeepRunning()) {
    allocator *alloc = create_stack_allocator(Gb);
    benchmark::DoNotOptimize(alloc);
    destroy_allocator(alloc);
  }
}

static void stack_allocator_allocate_small(benchmark::State &state) {
  allocator *alloc = create_stack_allocator(Gb);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, 75);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void stack_allocator_allocate_mid(benchmark::State &state) {
  allocator *alloc = create_stack_allocator(Gb);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Kb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void stack_allocator_allocate_large(benchmark::State &state) {
  allocator *alloc = create_stack_allocator(Gb);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Mb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void stack_allocator_allocate_parts_a6_d0(benchmark::State &state) {
  allocator *alloc = create_stack_allocator(Gb * 5);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

static void stack_allocator_allocate_parts_a8_d1(benchmark::State &state) {
  allocator *alloc = create_stack_allocator(Gb * 5);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    deallocate(alloc, b5);

    blk b7 = allocate(alloc, Mb * 64 * 4);
    benchmark::DoNotOptimize(b7);
    blk b8 = allocate(alloc, Mb * 64 * 4);
    benchmark::DoNotOptimize(b8);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

static void pool_allocator_cd(benchmark::State &state) {
  while (state.KeepRunning()) {
    allocator *alloc = create_pool_allocator(Mb, 1024);
    benchmark::DoNotOptimize(alloc);
    destroy_allocator(alloc);
  }
}

static void pool_allocator_allocate_small(benchmark::State &state) {
  allocator *alloc = create_pool_allocator(128, 8196);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, 75);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void pool_allocator_allocate_mid(benchmark::State &state) {
  allocator *alloc = create_pool_allocator(2 * Kb, 8196);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Kb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void pool_allocator_allocate_large(benchmark::State &state) {
  allocator *alloc = create_pool_allocator(2 * Mb, 8196);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Mb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void pool_allocator_allocate_parts_a6_d0(benchmark::State &state) {
  allocator *alloc = create_pool_allocator(Mb * 16, 64);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b6);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

static void pool_allocator_allocate_parts_a8_d1(benchmark::State &state) {
  allocator *alloc = create_pool_allocator(Mb * 16, 64);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 8);
    benchmark::DoNotOptimize(b6);

    deallocate(alloc, b5);

    blk b7 = allocate(alloc, Mb * 4);
    benchmark::DoNotOptimize(b7);
    blk b8 = allocate(alloc, Mb * 4);
    benchmark::DoNotOptimize(b8);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_cd(benchmark::State &state) {
  while (state.KeepRunning()) {
    allocator *alloc = create_bitmapped_allocator(Mb * 16);
    benchmark::DoNotOptimize(alloc);
    destroy_allocator(alloc);
  }
}

static void bitmapped_allocator_allocate_small(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Kb * 256);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, 75);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_allocate_mid(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Kb * 256);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Kb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_allocate_large(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Kb * 256);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Mb);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_allocate_large_ext(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Kb * 256);
  while (state.KeepRunning()) {
    auto blk = allocate(alloc, Mb * 2);
    benchmark::DoNotOptimize(blk);
    deallocate(alloc, blk);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_allocate_parts_a6_d0(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

static void bitmapped_allocator_allocate_parts_a8_d1(benchmark::State &state) {
  allocator *alloc = create_bitmapped_allocator(Mb * 64);
  while (state.KeepRunning()) {
    blk b1 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b1);
    blk b2 = allocate(alloc, Mb * 64 * 16);
    benchmark::DoNotOptimize(b2);
    blk b3 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b3);
    blk b4 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b4);
    blk b5 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b5);
    blk b6 = allocate(alloc, Mb * 64 * 8);
    benchmark::DoNotOptimize(b6);

    deallocate(alloc, b5);

    blk b7 = allocate(alloc, Mb * 64 * 4);
    benchmark::DoNotOptimize(b7);
    blk b8 = allocate(alloc, Mb * 64 * 4);
    benchmark::DoNotOptimize(b8);

    reset_allocator(alloc);
  }
  destroy_allocator(alloc);
}

BENCHMARK(malloc_allocate_small);
BENCHMARK(malloc_allocate_mid);
BENCHMARK(malloc_allocate_large);
BENCHMARK(malloc_allocate_large_a6_d0);
BENCHMARK(malloc_allocate_large_a8_d1);
BENCHMARK(stack_allocator_cd);
BENCHMARK(stack_allocator_allocate_small);
BENCHMARK(stack_allocator_allocate_mid);
BENCHMARK(stack_allocator_allocate_large);
BENCHMARK(stack_allocator_allocate_parts_a6_d0);
BENCHMARK(stack_allocator_allocate_parts_a8_d1);
BENCHMARK(pool_allocator_cd);
BENCHMARK(pool_allocator_allocate_small);
BENCHMARK(pool_allocator_allocate_mid);
BENCHMARK(pool_allocator_allocate_large);
BENCHMARK(pool_allocator_allocate_parts_a6_d0);
BENCHMARK(pool_allocator_allocate_parts_a8_d1);
BENCHMARK(bitmapped_allocator_cd);
BENCHMARK(bitmapped_allocator_allocate_small);
BENCHMARK(bitmapped_allocator_allocate_mid);
BENCHMARK(bitmapped_allocator_allocate_large);
BENCHMARK(bitmapped_allocator_allocate_large_ext);
BENCHMARK(bitmapped_allocator_allocate_parts_a6_d0);
BENCHMARK(bitmapped_allocator_allocate_parts_a8_d1);

BENCHMARK_MAIN();
