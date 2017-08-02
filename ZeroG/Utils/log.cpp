#include "log.h"

#include <cstdio>

void logger::write_log(char *message, int32_t length) {
  fwrite(message, length, 1, stdout);
}

void logger::write_err(char *message, int32_t length) {
  fwrite(message, length, 1, stderr);
}
