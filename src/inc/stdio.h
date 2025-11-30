#ifndef HLOS_STDIO_H
#define HLOS_STDIO_H

#include <stdarg.h>

int vsprintf(char *buf, const char *fmt, va_list args);

#endif