#ifndef CUSTOMASSERT
#define CUSTOMASSERT

#include "slog.h"

#ifdef DEBUG
    #define Assert(x)  if((x) == 0){\
        slog_fatal(0, "%s:%d:Assertion failed: %s", __FILE__, __LINE__, x);\
        exit(1);}
    #define LoggedAssert(x, ...)  if((x) == 0){\
        slog_fatal(0, "%s:%d:Assertion failed: %s", __FILE__, __LINE__, x);\
        slog_fatal(0, __VA_ARGS__ );\
        exit(1);}

#else
    #define Assert(x)
    #define LoggedAssert(x, y)
#endif

#endif
