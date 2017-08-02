#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "zerog_def.h"

namespace common {
struct ShaderSource {
  const char *source;
  enum { VERTEX = 0x8B31, FRAGMENT = 0x8B30, GEOMETRY = 0x8DD9 } type;
};

struct UniformBlock {
  const char *block_name;
  uint32_t block_size;
  uint32_t bind_point;
};

struct VertexAttr {
  enum { INT = 0x1404, UINT = 0x1405, FLOAT = 0x1406 } type;
  int32_t count;
  uint8_t normalized : sizeof(int32_t);
};

struct VertexBlock {
  int32_t vertex_size;
  int32_t vertex_count;
  VertexAttr *attribute_arr;
  int32_t attribute_arr_count;
  int32_t index_count;
} attr_packed;

struct PiplineCreateInfo {
  ShaderSource *shader_arr;
  UniformBlock *uniform_arr;
  VertexBlock *vertex_block;
  int32_t shader_arr_count;
  int32_t uniform_arr_count;
} attr_packed;
}

#endif // COMMON_TYPES_H
