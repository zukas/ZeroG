#include "engine.h"

#include "Renderer/renderer.h"
#include "internal/pipeline_store.h"

bool engine::init() {
  todo(allow variable pipeline store size);
  return renderer::init() && pipeline_store::init(16);
}

void engine::denint() {
  pipeline_store::deinit();
  renderer::deinit();
}

int32_t engine::pipeline::create(
    const common::PiplineCreateInfo *pipeline_create_info) {

  pipeline_store::pipeline temp_pipeline;

  temp_pipeline.program_id = renderer::create_shader_program(
      pipeline_create_info->shader_arr, pipeline_create_info->shader_arr_count);

  int32_t buffer_count =
      pipeline_create_info->uniform_arr_count +
      (pipeline_create_info->vertex_block->index_count > 0 ? 1 : 0) + 1;

  renderer::create_vertex_array(&temp_pipeline.vertex_array_id, 1);

  todo(use local stack allocator);
  uint32_t buffer_array[10];
  renderer::create_buffers(buffer_array, buffer_count);

  int32_t buffer_index = 0;
  temp_pipeline.vertex_data_buffer = buffer_array[buffer_index++];

  temp_pipeline.vertex_index_buffer =
      pipeline_create_info->vertex_block->index_count > 0
          ? buffer_array[buffer_index++]
          : 0;
  todo(use local stack allocator);
  pipeline_store::uniform_blk uniforms[8];
  for (int32_t i = 0; i < pipeline_create_info->uniform_arr_count; i++) {
    uint32_t block_buffer = buffer_array[buffer_index++];
    uint32_t block_index =
        renderer::bind_uniform_block(temp_pipeline.program_id, block_buffer,
                                     &pipeline_create_info->uniform_arr[i]);
    uniforms[i].name_hash =
        common::hash(pipeline_create_info->uniform_arr[i].block_name);
    uniforms[i].block_index = block_index;
    uniforms[i].block_buffer = block_buffer;
  }
  temp_pipeline.uniform_data_arr = uniforms;
  temp_pipeline.uniform_data_arr_count =
      pipeline_create_info->uniform_arr_count;

  renderer::bind_vertex_array(
      temp_pipeline.vertex_array_id, temp_pipeline.vertex_data_buffer, nullptr,
      pipeline_create_info->vertex_block->vertex_size *
          pipeline_create_info->vertex_block->vertex_count,
      pipeline_create_info->vertex_block->vertex_size,
      pipeline_create_info->vertex_block->attribute_arr,
      pipeline_create_info->vertex_block->attribute_arr_count);
  if (temp_pipeline.vertex_index_buffer != 0) {
    renderer::bind_index_array(temp_pipeline.vertex_array_id,
                               temp_pipeline.vertex_index_buffer, nullptr,
                               pipeline_create_info->vertex_block->index_count *
                                   size_of<int32_t>());
  }

  temp_pipeline.vertex_array_count =
      pipeline_create_info->vertex_block->vertex_count;
  temp_pipeline.vertex_index_count =
      pipeline_create_info->vertex_block->index_count;

  return pipeline_store::store(&temp_pipeline);
}

void engine::pipeline::destroy(int32_t pipeline_id) {}

void engine::pipeline::set_vertex_data(int32_t pipeline_id, int32_t offset,
                                       int32_t size, const void *data) {
  auto pipeline = pipeline_store::get(pipeline_id);
  renderer::update_vertex_array(pipeline->vertex_array_id,
                                pipeline->vertex_data_buffer, data, size,
                                offset);
}

void engine::pipeline::set_vertex_index_data(uint32_t pipeline_id,
                                             int32_t offset, int32_t size,
                                             const void *data) {
  auto pipeline = pipeline_store::get(pipeline_id);
  renderer::update_index_array(pipeline->vertex_array_id,
                               pipeline->vertex_index_buffer, data, size,
                               offset);
}

void engine::pipeline::set_uniform_data(uint32_t pipeline_id,
                                        uint32_t uniform_name_hash,
                                        int32_t offset, int32_t size,
                                        const void *data) {
  auto pipeline = pipeline_store::get(pipeline_id);
  for (int32_t i = 0; i < pipeline->uniform_data_arr_count; i++) {
    if (pipeline->uniform_data_arr[i].name_hash == uniform_name_hash) {
      renderer::update_uniform_buffer(
          pipeline->uniform_data_arr[i].block_buffer, data, size, offset);
      break;
    }
  }
}

void engine::commit_to_frame(const int32_t *pipeline_arr,
                             int32_t pipeline_arr_count) {
  for (int32_t i = 0; i < pipeline_arr_count; i++) {
    auto pipeline = pipeline_store::get(pipeline_arr[i]);
    renderer::use_program(pipeline->program_id);
    renderer::draw_vertex_array(pipeline->vertex_array_id,
                                pipeline->vertex_array_count);
  }
}
