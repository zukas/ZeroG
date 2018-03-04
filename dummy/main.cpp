#include "../zerog/engine.h"

int main() {

  ZeroG::app_info info;
  info.app_name = "My Game";
  info.app_version = ZeroG::make_version(0, 0, 1);
  ZeroG::engine *e = ZeroG::init_engine(&info);

  ZeroG::deinit_engine(e);

  return 0;
}
