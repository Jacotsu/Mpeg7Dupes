#ifndef CUSTOMASSERT
#define CUSTOMASSERT

#include <stdarg.h>

#include "slog.h"

// 0    panic
// 1    fatal
// 2    error
// 3    warn
// 4    info
// 5    live
// 6    debug
#define Assert(x)  if((x) == 0){\
    slog_fatal(1, "Assertion failed: %s",  #x);\
    exit(1);}
#define LoggedAssert(x, ...)  if((x) == 0){\
    slog_fatal(1, "Assertion failed: %s", #x);\
    slog_fatal(1, __VA_ARGS__ );\
    exit(1);}

#endif

#define AssertFileExistence(x, ...)  {\
    FILE *__fileToAssert = fopen(x, "rb");\
    LoggedAssert(__fileToAssert, __VA_ARGS__);\
    fclose(__fileToAssert);\
    exit(1);}
