#ifndef MAIN
#define MAIN

#define DEBUG

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#define COARSE_SIZE 90
#define SIGELEM_SIZE 380
#define MAX_FRAMERATE 60
#define HOUGH_MAX_OFFSET 90
#define DIR_NEXT 1
#define DIR_NEXT_END 3
#define DIR_PREV 0
#define DIR_PREV_END 2
#define STATUS_NULL 0
#define STATUS_END_REACHED 1
#define STATUS_BEGIN_REACHED 2

// ffmpeg downloaded headers
#include "get_bits.h"
#include "signature.h"

// custom headers
#include "slog.h"
#include "customAssert.h"
#include "ArgumentParsing.h"



#define FFMAX(a,b) ((a) > (b) ? (a) : (b))


typedef struct BoundedCoarseSignature {
    // StartFrameOfSegment and EndFrameOfSegment
    uint32_t firstIndex, lastIndex;
    // StartMediaTimeOfSegment and EndMediaTimeOfSegment
    uint64_t firstPts, lastPts;
    CoarseSignature *cSign;
} BoundedCoarseSignature;


StreamContext* binary_import(const char*);

int fineSignatureCmp(const void*, const void*);

#endif
