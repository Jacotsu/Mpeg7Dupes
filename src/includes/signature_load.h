#ifndef SIGNATURE_LOAD
#define SIGNATURE_LOAD

#define SIGELEM_SIZE 380

#include <stdio.h>
#include <stdlib.h>


// FFMPEG downloaded headers
#include "get_bits.h"
#include "signature.h"

// Libraries
#include "slog.h"

// Custom headers
#include "utils.h"
#include "customAssert.h"
#include "ArgumentParsing.h"
#include "signature_load.h"


void
binary_import(StreamContext*,const char*);

#endif
