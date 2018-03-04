#ifndef INFO_H
#define INFO_H

#include <cstdint>

namespace ZeroG {
constexpr uint32_t make_version(uint32_t major, uint32_t minor,
                                uint32_t patch) {
  return (major << 22) | (minor << 12) | patch;
}

constexpr const char *ENGIEN_NAME{"ZeroG"};
constexpr uint32_t ENGINE_VERSION{make_version(0, 0, 1)};

struct app_info {
  const char *app_name;
  uint32_t app_version;
};
} // namespace ZeroG

#endif // INFO_H
