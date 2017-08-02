#ifndef RENDERER_H
#define RENDERER_H

#include "../Common/types.h"
#include "common.h"

#include <glm/glm.hpp>

namespace renderer {
bool init();
void deinit();

Rect get_viewport();

void enable_cursor(bool enable = true);
void enable_sticky_keys(bool enable = true);
void enable_sticky_mouse(bool enable = true);

void set_clear_colour(glm::vec4 colour);

void set_clear_depth(float depth);
void set_depth_lt();
void set_depth_gt();
void set_depth_eq();

void enable_feature(Feature feature);
void disable_feature(Feature feature);

uint32_t create_shader_program(const common::ShaderSource *shaders,
                               int32_t size);
void destroy_shader_program(uint32_t shader_program_id);
void use_program(uint32_t program_id);

void create_buffers(uint32_t *buffers, int32_t size);
void destry_buffers(const uint32_t *buffers, int32_t size);

void create_vertex_array(uint32_t *vertex_array, int32_t size);
void destry_vertex_array(const uint32_t *vertex_array, int32_t size);

uint32_t resolve_uniform_block_index(uint32_t program_id,
                                     const char *block_name);

void bind_uniform_block(uint32_t program_id, uint32_t buffer_id,
                        uint32_t block_index, uint32_t bind_point,
                        const void *data, uint32_t size);

uint32_t bind_uniform_block(uint32_t program_id, uint32_t buffer_id,
                            const common::UniformBlock *uniform_block);

void update_uniform_buffer(uint32_t buffer_id, const void *data, int32_t size,
                           int32_t offset);

void bind_vertex_array(uint32_t vertex_array, uint32_t vertex_buffer,
                       const void *data, int32_t size, int32_t elem_size,
                       const common::VertexAttr *attr, int32_t attr_count);

void update_vertex_array(uint32_t vertex_array, uint32_t vertex_buffer,
                         const void *data, int32_t size, int32_t offset);

void bind_index_array(uint32_t vertex_array, uint32_t index_buffer,
                      const void *data, int32_t size);
void update_index_array(uint32_t vertex_array, uint32_t index_buffer,
                        const void *data, int32_t size, int32_t offset);

void draw_vertex_array(uint32_t vertex_array, int32_t vertex_count);
void draw_vertex_array_indexed(uint32_t vertex_array, int32_t elem_count);

void begin_frame();

bool end_frame();
}

#endif // RENDERER_H
