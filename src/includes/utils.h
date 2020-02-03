#ifndef UTILS
#define UTILS

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


#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

#include <string.h>
#include <stdio.h>
#include <math.h>

// ffmpeg downloaded headers
#include "signature.h"

// custom headers
#include "slog.h"
#include "customAssert.h"



typedef struct BoundedCoarseSignature {
    // StartFrameOfSegment and EndFrameOfSegment
    uint32_t firstIndex, lastIndex;
    // StartMediaTimeOfSegment and EndMediaTimeOfSegment
    uint64_t firstPts, lastPts;
    CoarseSignature *cSign;
} BoundedCoarseSignature;


char*
padStr(char *str, char *buffer, int maxLen, char padChar);

void
printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse);

void
printCoarseSigList(CoarseSignature *list);

void
printStreamContext(StreamContext *sc);

int
fineSignatureCmp(const void *p1, const void *p2);

#endif
