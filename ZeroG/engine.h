#ifndef ENGINE_H
#define ENGINE_H

#include "Common/hash.h"
#include "Common/types.h"

namespace engine {
bool init();
void denint();

namespace pipeline {
int32_t create(const common::PiplineCreateInfo *pipeline_create_info);
void destroy(int32_t pipeline_id);
void set_vertex_data(int32_t pipeline_id, int32_t offset, int32_t size,
                     const void *data);
void set_vertex_index_data(uint32_t pipeline_id, int32_t offset, int32_t size,
                           const void *data);
void set_uniform_data(uint32_t pipeline_id, uint32_t uniform_name_hash,
                      int32_t offset, int32_t size, const void *data);
}

todo(change temporaty commit to frame);
void commit_to_frame(const int32_t *pipeline_arr, int32_t pipeline_arr_count);
}

#endif // ENGINE_H
