#include <benchmark/benchmark.h>
#include <chrono>
#include <mutex>
#include <random>

//#include <gperftools/tcmalloc.h>

#include <cstdlib>
#include <cstring>

#include "../ZeroG/Utils/memz.h"
#include "../ZeroG/Utils/memz2.h"

#define ELEM_COUNT 1000

class malloc_test : public ::benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State &st) {
    std::lock_guard<std::mutex> g(lock);
    stride = st.range(2);
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(void *));
    dist = std::uniform_int_distribution<size_t>{
        static_cast<size_t>(st.range(0)), static_cast<size_t>(st.range(1))};
    for (int i = 0; i < ELEM_COUNT; ++i) {
      sizes[i] = dist(gen);
    }
  }
  void TearDown(const ::benchmark::State &) {
    std::lock_guard<std::mutex> g(lock);
    for (int i = 0; i < ELEM_COUNT; i++) {
      if (data[i] != nullptr)
        deallocate(data[i]);
    }
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(void *));
  }

  void *allocate(size_t size) { return ::malloc(size); }

  void deallocate(void *ptr) { ::free(ptr); }

  long stride;
  size_t sizes[ELEM_COUNT];
  void *data[ELEM_COUNT];

private:
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<size_t> dist{};
  std::mutex lock;
};

BENCHMARK_DEFINE_F(malloc_test, obj)(benchmark::State &state) {
  while (state.KeepRunning()) {
    for (long i = 0; i < stride; i++) {
      for (long j = i % stride; j < ELEM_COUNT; j += stride) {
        void *ptr = allocate(sizes[j]);
        if (data[i]) {
          deallocate(data[i]);
        }
        data[i] = ptr;
      }
    }
  }
}

class memz_test : public ::benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State &st) {
    std::lock_guard<std::mutex> g(lock);

    memz::init(1024ul * 1024ul * 1024ul * 3ul);

    stride = st.range(2);
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(void *));
    dist = std::uniform_int_distribution<size_t>{
        static_cast<size_t>(st.range(0)), static_cast<size_t>(st.range(1))};
    for (int i = 0; i < ELEM_COUNT; ++i) {
      sizes[i] = dist(gen);
    }
  }
  void TearDown(const ::benchmark::State &) {
    std::lock_guard<std::mutex> g(lock);
    for (long i = 0; i < ELEM_COUNT; i++) {
      if (data[i].ptr) {
        memz::dealloc(data[i]);
      }
    }
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(memz::Blk));
    memz::deinit();
  }

  memz::Blk allocate(size_t size) { return memz::alloc(size); }

  void deallocate(memz::Blk blk) { memz::dealloc(blk); }

  long stride;
  size_t sizes[ELEM_COUNT];
  memz::Blk data[ELEM_COUNT];

private:
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<size_t> dist{};
  std::mutex lock;
};

BENCHMARK_DEFINE_F(memz_test, obj)(benchmark::State &state) {
  while (state.KeepRunning()) {
    for (long i = 0; i < stride; i++) {
      for (long j = i % stride; j < ELEM_COUNT; j += stride) {
        memz::Blk blk = allocate(sizes[j]);
        if (data[i].ptr) {
          deallocate(data[i]);
        }
        data[i] = blk;
      }
    }
  }
}

class memz2_test : public ::benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State &st) {
    std::lock_guard<std::mutex> g(lock);

    memz2::AllocatorCreateInfo info;
    info.type = memz2::AllocatorType::BLOCK;
    info.size = 1024ul * 1024ul * 1024ul * 3ul;
    info.parent_allocator = nullptr;

    alloc = memz2::create_allocator(&info);

    stride = st.range(2);
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(void *));
    dist = std::uniform_int_distribution<size_t>{
        static_cast<size_t>(st.range(0)), static_cast<size_t>(st.range(1))};
    for (int i = 0; i < ELEM_COUNT; ++i) {
      sizes[i] = dist(gen);
    }
  }
  void TearDown(const ::benchmark::State &) {
    std::lock_guard<std::mutex> g(lock);
    for (long i = 0; i < ELEM_COUNT; i++) {
      if (data[i]) {
        memz2::dealloc(alloc, data[i]);
      }
    }
    memset(sizes, 0, ELEM_COUNT * sizeof(size_t));
    memset(data, 0, ELEM_COUNT * sizeof(memz::Blk));
    memz::deinit();
  }

  void *allocate(size_t size) { return memz2::alloc(alloc, size); }

  void deallocate(void *blk) { memz2::dealloc(alloc, blk); }

  long stride;
  memz2::Allocator alloc;
  size_t sizes[ELEM_COUNT];
  void *data[ELEM_COUNT];

private:
  std::random_device rd;
  std::mt19937 gen{rd()};
  std::uniform_int_distribution<size_t> dist{};
  std::mutex lock;
};

BENCHMARK_DEFINE_F(memz2_test, obj)(benchmark::State &state) {
  while (state.KeepRunning()) {
    for (long i = 0; i < stride; i++) {
      for (long j = i % stride; j < ELEM_COUNT; j += stride) {
        void *blk = allocate(sizes[j]);
        if (data[i]) {
          deallocate(data[i]);
        }
        data[i] = blk;
      }
    }
  }
}

#define TEST_FIXTURE(fixture, MIN, MAX)                                        \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 2});                     \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 7});                     \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 49});                    \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 113});                   \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 371});                   \
  BENCHMARK_REGISTER_F(fixture, obj)->Args({MIN, MAX, 913});

#define FULL_BENCHMARK_SET(MIN, MAX, THREADS)                                  \
  TEST_FIXTURE(malloc_test, MIN, MAX)                                          \
  TEST_FIXTURE(memz_test, MIN, MAX)                                            \
  TEST_FIXTURE(memz2_test, MIN, MAX)

FULL_BENCHMARK_SET(8, 256, 1)
FULL_BENCHMARK_SET(257, 1024, 1)
FULL_BENCHMARK_SET(1025, 8196, 1)
FULL_BENCHMARK_SET(8197, 32784, 1)
FULL_BENCHMARK_SET(32785, 131136, 1)

// FULL_BENCHMARK_SET(8, 256, 2)
// FULL_BENCHMARK_SET(257, 1024, 2)
// FULL_BENCHMARK_SET(1025, 8196, 2)
// FULL_BENCHMARK_SET(8197, 32784, 2)
// FULL_BENCHMARK_SET(32785, 131136, 2)

// FULL_BENCHMARK_SET(8, 256, 4)
// FULL_BENCHMARK_SET(257, 1024, 4)
// FULL_BENCHMARK_SET(1025, 8196, 4)
// FULL_BENCHMARK_SET(8197, 32784, 4)
// FULL_BENCHMARK_SET(32785, 131136, 4)

// FULL_BENCHMARK_SET(8, 256, 8)
// FULL_BENCHMARK_SET(257, 1024, 8)
// FULL_BENCHMARK_SET(1025, 8196, 8)
// FULL_BENCHMARK_SET(8197, 32784, 8)
// FULL_BENCHMARK_SET(32785, 131136, 8)

BENCHMARK_MAIN();
