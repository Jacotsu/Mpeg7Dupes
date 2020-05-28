#ifndef MAIN
#define MAIN


#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// ffmpeg downloaded headers
#include "get_bits.h"
#include "signature.h"
#include "signature_load.h"

// custom headers
#include "debug.h"
#include "slog.h"
#include "customAssert.h"
#include "printers.h"
#include "ArgumentParsing.h"
#include "utils.h"

#define NUM_OF_INPUTS 2

void
processFiles(struct fileIndex *index, void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne), int useOpenMp);

void
processFilePair(struct fileIndex *index, void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne));


#endif
