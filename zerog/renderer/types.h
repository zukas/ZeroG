#ifndef VK_H
#define VK_H

#include <cstddef>
#include <cstdint>

namespace ZeroG {

struct WindowCreateInfo {
  const char *caption;
  int32_t width;
  int32_t height;
  int32_t full_screen;
};

struct AppCreateInfo {
  const char *app_name;
  const char *engine_name;
  uint32_t app_version;
  uint32_t engien_version;
};

struct KernelCreateInfo {
  WindowCreateInfo *window_info;
  AppCreateInfo *app_info;
};

enum DescriptorType {
  SAMPLER = 0,
  COMBINED_IMAGE_SAMPLER = 1,
  SAMPLED_IMAGE = 2,
  STORAGE_IMAGE = 3,
  UNIFORM_TEXEL_BUFFER = 4,
  STORAGE_TEXEL_BUFFER = 5,
  UNIFORM_BUFFER = 6,
  STORAGE_BUFFER = 7,
  UNIFORM_BUFFER_DYNAMIC = 8,
  STORAGE_BUFFER_DYNAMIC = 9,
  INPUT_ATTACHMENT = 10
};

enum ShaderStage {
  VERTEX = 0x00000001,
  TESSELLATION_CONTROL = 0x00000002,
  TESSELLATION_EVALUATION = 0x00000004,
  GEOMETRY = 0x00000008,
  FRAGMENT = 0x00000010,
  COMPUTE = 0x00000020,
  ALL_GRAPHICS = 0x0000001F,
  ALL = 0x7FFFFFFF
};

struct DescriptorSetCreateInfo {
  DescriptorType *types;
  ShaderStage *stages;
  uint32_t descriptor_count;
};

enum DataInputRate { PER_VERTEX = 0, PER_INSTANCE = 1 };

enum DataType {
  UINT = 98,
  INT = 99,
  FLOAT = 100,
  VEC2 = 103,
  VEC3 = 106,
  VEC4 = 109
};

struct DataAttribute {
  DataType type;
  uint32_t offset;
};

struct DataBinding {
  DataInputRate rate;
  DataAttribute *attributes;
  uint32_t attr_count;
  uint32_t stride;
};

struct DataLayoutCreateInfo {
  DataBinding *bindings;
  uint32_t binding_count;
};

struct Shader {
  uint32_t *code;
  size_t length;
  ShaderStage stage;
};

struct PipelineCreateInfo {
  DescriptorSetCreateInfo *descriptor_info;
  DataLayoutCreateInfo *data_layout_info;
  Shader *shaders;
  uint32_t shader_count;
};

} // namespace ZeroG

#endif // VK_H
