#ifndef MAIN
#define MAIN

#define DEBUG
#ifdef DEBUG
int __DEBUG = 1;
#else
int __DEBUG = 0;
#endif


#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ffmpeg downloaded headers
#include "get_bits.h"
#include "signature.h"
#include "signature_load.h"

// custom headers
#include "slog.h"
#include "customAssert.h"
#include "ArgumentParsing.h"


#endif
