
#include "../ZeroG/Renderer/renderer.h"
#include "../ZeroG/engine.h"

#include <glm/gtc/matrix_transform.hpp>

constexpr const char *vertex_shader{
    "#version 450\n"
    "layout(location = 0) in vec2 in_position;"
    "layout(location = 1) in vec3 in_colour;"
    "layout(location = 0, std140) uniform MP {"
    "mat4 proj;"
    "mat4 model;"
    "} mp;"
    "out vec3 st_colour;"
    "void main() {"
    "st_colour = in_colour;"
    "mat4 local_mp = mp.proj * mp.model;"
    "gl_Position = local_mp * vec4(in_position, 1.0, 1.0);"
    "}"};

constexpr const char *fragment_shader{"#version 450\n"
                                      "in vec3 st_colour;"
                                      "out vec4 out_colour;"
                                      "void main() {"
                                      "out_colour = vec4(st_colour, 1.0);"
                                      "}"};

struct MP {
  glm::mat4 proj;
  glm::mat4 model;
};

struct vertex {
  glm::vec2 pos;
  glm::vec3 colour;
};

int main() {

  engine::init();

  renderer::set_clear_colour({242, 116, 275, 255});
  renderer::set_clear_depth(1.f);
  renderer::disable_feature(renderer::DEPTH_TEST);

  common::ShaderSource shaders[2]{
      {vertex_shader, common::ShaderSource::VERTEX},
      {fragment_shader, common::ShaderSource::FRAGMENT}};

  common::UniformBlock uniforms[]{{"MP", sizeof(MP), 0}};

  common::VertexAttr attrs[2]{{common::VertexAttr::FLOAT, 2, false},
                              {common::VertexAttr::FLOAT, 3, false}};

  common::VertexBlock vertex_block;
  vertex_block.vertex_size = sizeof(vertex);
  vertex_block.vertex_count = 6;
  vertex_block.index_count = 0;
  vertex_block.attribute_arr = attrs;
  vertex_block.attribute_arr_count = 2;

  common::PiplineCreateInfo create_info;
  create_info.shader_arr = shaders;
  create_info.shader_arr_count = 2;
  create_info.uniform_arr = uniforms;
  create_info.uniform_arr_count = 1;
  create_info.vertex_block = &vertex_block;

  auto pipeline = engine::pipeline::create(&create_info);

  auto viewport = renderer::get_viewport();

  vertex vertexes[6]{
      {{0.f, 0.f}, {0.f, 0.f, 0.f}},   {{250.f, 0.f}, {1.f, 0.f, 0.f}},
      {{0.f, 200.f}, {0.f, 1.f, 0.f}}, {{250.f, 200.f}, {1.f, 1.f, 0.f}},
      {{0.f, 200.f}, {0.f, 1.f, 0.f}}, {{250.f, 0.f}, {1.f, 0.f, 0.f}}};

  MP mp{glm::ortho(viewport.x, viewport.w, viewport.y, viewport.h),
        glm::translate(glm::mat4(1.0), glm::vec3{100, 30, 0})};

  engine::pipeline::set_vertex_data(pipeline, 0, sizeof(vertexes), &vertexes);
  engine::pipeline::set_uniform_data(pipeline, "MP"_h, 0, sizeof(mp), &mp);

  float offset = 0.f;
  do {
    renderer::begin_frame();
    engine::commit_to_frame(&pipeline, 1);
    //    renderer::draw_vertex_array(vertex_arrays[0], 6);
    offset += 1;
    mp.model[3][0] = offset;
    engine::pipeline::set_uniform_data(pipeline, "MP"_h, 0, sizeof(mp), &mp);
  } while (renderer::end_frame() && offset < viewport.w);

  renderer::deinit();

  return 0;
}
