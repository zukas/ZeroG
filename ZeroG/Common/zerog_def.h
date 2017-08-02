#ifndef ZEROG_DEF_H
#define ZEROG_DEF_H

#include <cstdint>

#define attr_packed __attribute__((packed))

#define cpp_do_pragma(x) _Pragma(#x)
#define todo(x) cpp_do_pragma(message("TODO - " #x))

template <typename T, typename _Res = int32_t> _Res size_of() {
  return static_cast<_Res>(sizeof(T));
}

#endif // ZEROG_DEF_H
