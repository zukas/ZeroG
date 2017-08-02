#ifndef COMMON_H
#define COMMON_H

#include <cstddef>
#include <cstdint>

namespace renderer {
enum Feature {
  BLEND = 0x0BE2,
  CULL_FACE = 0x0B44,
  DEPTH_TEST = 0x0B71,
  LINE_SMOOTH = 0x0B20,
  MULTISAMPLE = 0x809D,
  POLYGON_OFFSET_FILL = 0x8037,
  POLYGON_OFFSET_LINE = 0x2A02,
  POLYGON_OFFSET_POINT = 0x2A01,
  POLYGON_SMOOTH = 0x0B41,
  SCISSOR_TEST = 0x0C11,
  STENCIL_TEST = 0x0B90
};

struct Rect {
  float x;
  float y;
  float w;
  float h;
};
}

#endif // COMMON_H
