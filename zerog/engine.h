#ifndef ENGINE_H
#define ENGINE_H

#include "info.h"

namespace ZeroG {

struct engine;

engine *init_engine(app_info *app);
void deinit_engine(engine *instance);

} // namespace ZeroG

#endif // ENGINE_H
