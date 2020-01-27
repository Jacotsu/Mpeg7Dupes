#ifndef CUSTOMASSERT
#define CUSTOMASSERT

#include "slog.h"

#ifdef DEBUG
    #define Assert(x)  if((x) == 0){\
        slog_fatal(0, "%s:%d Assertion failed: %s", __FILE__, __LINE__, x);\
        /*some error handling*/}
    #define LoggedAssert(x, y)  if((x) == 0){\
        slog_fatal(0, "%s:%d Assertion failed: %s", __FILE__, __LINE__, x);\
        slog_fatal(0, "%s", y);\
        /*some error handling*/}

#else
    #define Assert(x)
    #define LoggedAssert(x, y)
#endif

#endif
