#include "pipeline_store.h"
#include "../Common/zerog_def.h"
#include "../Utils/memz2.h"

todo(do proper heap management);
struct {

  pipeline_store::pipeline pipeline_arr[16];
  int32_t pipeline_arr_count;
  pipeline_store::uniform_blk uniform_block_arr[32];
  int32_t uniform_block_arr_count;
} __pipeline_store;

bool pipeline_store::init(int32_t size) {
  __pipeline_store.pipeline_arr_count = 0;
  __pipeline_store.uniform_block_arr_count = 0;
}

void pipeline_store::deinit() {}

int32_t pipeline_store::store(const pipeline_store::pipeline *pipeline) {

  int32_t p_idx = __pipeline_store.pipeline_arr_count;
  int32_t u_idx = __pipeline_store.uniform_block_arr_count;

  ++__pipeline_store.pipeline_arr_count;
  __pipeline_store.uniform_block_arr_count += pipeline->uniform_data_arr_count;

  __pipeline_store.pipeline_arr[p_idx] = *pipeline;
  __pipeline_store.pipeline_arr[p_idx].uniform_data_arr =
      &__pipeline_store.uniform_block_arr[u_idx];

  for (int32_t i = 0; i < pipeline->uniform_data_arr_count; i++) {
    __pipeline_store.uniform_block_arr[u_idx + i] =
        pipeline->uniform_data_arr[i];
  }

  return p_idx;
}

void pipeline_store::remove(int32_t pipeline_id) {}

const pipeline_store::pipeline *pipeline_store::get(int32_t pipeline_id) {
  return &__pipeline_store.pipeline_arr[pipeline_id];
}
