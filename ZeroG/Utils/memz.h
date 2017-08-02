#ifndef MEMZ_H
#define MEMZ_H

#include <cstddef>

namespace memz {
struct Blk {
  void *ptr;
  size_t size;
};

void init(size_t size);
void deinit();
Blk alloc(size_t size);
Blk realloc(Blk blk, size_t size);
Blk aligned(size_t alignment, size_t size);
void dealloc(Blk blk);

template <typename T> class ptr {
private:
  T *_ptr;
  size_t _size;

public:
  ptr() {
    auto blk = alloc(sizeof(T));
    _ptr = static_cast<T *>(blk.ptr);
    _size = blk.size;
  }
  ~ptr() { dealloc({_ptr, _size}); }

  T *operator->() { return _ptr; }
  const T *operator->() const { return _ptr; }
};

template <typename T> class array {
private:
  T *_ptr;
  size_t _size;
  size_t _count;

public:
  array() : _ptr(nullptr), _size(0), _count(0) {}
  array(size_t count) : _count(count) {
    auto blk = alloc(sizeof(T) * count);
    _ptr = static_cast<T *>(blk.ptr);
    _size = blk.size;
  }
  ~array() {
    if (_ptr)
      dealloc({_ptr, _size});
  }

  size_t count() { return _count; }

  T *operator->() { return _ptr; }
  const T *operator->() const { return _ptr; }

  T &operator[](size_t idx) { return _ptr[idx]; }
  const T &operator[](size_t idx) const { return _ptr[idx]; }
};
}

#endif // MEMZ_H
