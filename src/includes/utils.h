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

#define MAX_PATH_LENGTH 320


#include <string.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

// ffmpeg downloaded headers
#include "macros.h"
#include "signature.h"

// custom headers
#include "slog.h"
#include "customAssert.h"


struct fileIndex {
    long int indexA, indexB, maxIndexA, maxIndexB;
    char *pathsMatrix;
};

typedef struct BoundedCoarseSignature {
    // StartFrameOfSegment and EndFrameOfSegment
    uint32_t firstIndex, lastIndex;
    // StartMediaTimeOfSegment and EndMediaTimeOfSegment
    uint64_t firstPts, lastPts;
    CoarseSignature *cSign;
} BoundedCoarseSignature;

struct fileIndex
mergeFileIterators(struct fileIndex *, struct fileIndex *);

int
initFileIterator(struct fileIndex *fileIndex, char *fileListName);

int
initFileIteratorFromCmdLine(struct fileIndex *fileIndex,
	char **argv, int argc);

char *
getIteratorIndexFilePath(struct fileIndex *fileIndex, char indexSelector);

unsigned int
getNumberOfLinesFromFilename(char *filename);

int
terminateFileIterator(struct fileIndex *fileIndex);

int
nextFileIteration(struct fileIndex *fileIndex);

int
nextFileIterationByIndex(struct fileIndex *fileIndex, char index);

char*
padStr(char *str, char *buffer, int maxLen, char padChar);

int
fineSignatureCmp(const void *p1, const void *p2);

unsigned int
getFileSize(const char *filename);

unsigned int
getPathLastSlashPosition(const char *path);

unsigned int
buildDirectoryTree(const char *path);

#endif
