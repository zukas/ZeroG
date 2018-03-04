#include "engine.h"

#include "common/hash.h"
#include "renderer/renderer.h"

#include "memory/memory.h"

namespace ZeroG {
struct engine {
  allocator *core_allocator;
  allocator *engine_allocator;
  renderer *render;
  size_t size;
};
} // namespace ZeroG

ZeroG::engine *ZeroG::init_engine(app_info *app) {
  allocator *core = create_bitmapped_allocator(Mb * 16);
  allocator *base = create_stack_allocator(Mb * 16, core);
  blk b = allocate(base, sizeof(engine));
  engine *e = static_cast<engine *>(b.ptr);
  e->core_allocator = core;
  e->engine_allocator = base;
  e->render = init_renderer(core);
  e->size = b.size;

  AppCreateInfo app_info;
  app_info.app_name = app->app_name;
  app_info.app_version = app->app_version;
  app_info.engine_name = ENGIEN_NAME;
  app_info.engien_version = ENGINE_VERSION;

  WindowCreateInfo window_info;
  window_info.full_screen = true;
  window_info.caption = app->app_name;
  window_info.width = -1;
  window_info.height = -1;

  KernelCreateInfo info;
  info.app_info = &app_info;
  info.window_info = &window_info;

  create_kernel(e->render, &info);

  return e;
}

void ZeroG::deinit_engine(ZeroG::engine *instance) {
  destry_kernel(instance->render);
  deinit_renderer(instance->render);
  destroy_allocator(instance->core_allocator);
}
