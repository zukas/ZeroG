#ifndef PIPELINE_STORE_H
#define PIPELINE_STORE_H

#include <cstdint>

namespace pipeline_store {

struct uniform_blk {
  uint32_t name_hash;
  uint32_t block_index;
  uint32_t block_buffer;
};

struct pipeline {
  uint32_t program_id;
  uint32_t vertex_array_id;
  int32_t vertex_array_count;
  uint32_t vertex_data_buffer;
  uint32_t vertex_index_buffer;
  int32_t vertex_index_count;
  uniform_blk *uniform_data_arr;
  int32_t uniform_data_arr_count;
};

bool init(int32_t size);
void deinit();

int32_t store(const pipeline *pipeline);
void remove(int32_t pipeline_id);
const pipeline *get(int32_t pipeline_id);
}

#endif // PIPELINE_STORE_H
