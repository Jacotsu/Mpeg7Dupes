#ifndef CUSTOMASSERT
#define CUSTOMASSERT

#include "slog.h"

#ifdef DEBUG
    #define Assert(x)  if((x) == 0){\
        slog_fatal(0, "Assertion failed: %s", x);\
        /*some error handling*/}
#else
    #define Assert(x)
#endif

#endif
