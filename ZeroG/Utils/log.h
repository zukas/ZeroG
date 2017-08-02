#ifndef LOG_H
#define LOG_H

#include <cstdint>

namespace logger {
void write_log(char *message, int32_t length);
void write_err(char *message, int32_t length);
}

#endif // LOG_H
