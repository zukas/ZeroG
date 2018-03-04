#include "renderer.h"

#include "vk/initializer.h"

namespace ZeroG {
PTR_DEFINITION(Kernel);
PTR_IMPLEMENTATION(Kernel);

struct renderer {
  allocator *renderer_allocator;
  Kernel kernel;
  size_t size;
};
} // namespace ZeroG

ZeroG::renderer *ZeroG::init_renderer(allocator *base) {
  allocator *r = create_bitmapped_allocator(Mb * 4, base);
  blk blk = allocate(r, sizeof(renderer));
  renderer *rend = static_cast<renderer *>(blk.ptr);

  rend->renderer_allocator = r;
  rend->size = blk.size;

  return rend;
}

void ZeroG::deinit_renderer(ZeroG::renderer *instance) {
  destroy_allocator(instance->renderer_allocator);
}

void ZeroG::create_kernel(renderer *instance,
                          const ZeroG::KernelCreateInfo *kinfo) {
  allocator *buffer = create_stack_allocator(Mb, instance->renderer_allocator);
  instance->kernel.window = vk::create_window(kinfo->window_info);
  instance->kernel.instance = vk::create_instance(buffer, kinfo->app_info);
  instance->kernel.surface = vk::create_surface(
      instance->kernel.instance.instance, instance->kernel.window);
  instance->kernel.physical_device = vk::select_physical_device(
      buffer, instance->kernel.instance.instance, instance->kernel.surface);
  instance->kernel.logical_device =
      vk::create_device(buffer, instance->kernel.physical_device);
  destroy_allocator(buffer);
}

void ZeroG::destry_kernel(renderer *instance) {
  vk::destroy_device(instance->kernel.logical_device);
  vk::destroy_surface(instance->kernel.instance.instance,
                      instance->kernel.surface);
  vk::destroy_instance(instance->kernel.instance);
  vk::destroy_window(instance->kernel.window);
}
