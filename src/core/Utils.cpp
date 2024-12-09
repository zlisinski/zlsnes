#include <stdarg.h>
#include <string>

#include "Utils.h"

std::string fmt(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);

    va_end(args);

    return std::string(buf);
}