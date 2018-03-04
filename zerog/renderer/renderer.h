#ifndef RENDERER_H
#define RENDERER_H

#include "types.h"

#include "memory/memory.h"

namespace ZeroG {

struct renderer;

renderer *init_renderer(allocator *base);
void deinit_renderer(renderer *instance);

void create_kernel(renderer *instance, const KernelCreateInfo *kinfo);
void destry_kernel(renderer *instance);

} // namespace ZeroG

#endif // RENDERER_H
