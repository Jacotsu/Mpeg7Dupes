#ifndef PRINTERS
#define PRINTERS

#include "signature.h"
#include "utils.h"

void
printBeautifulHeader();

void
printCSVHeader();

void
printBeautiful(MatchingInfo *info, StreamContext* sc, char *file1,\
	char *file2, int isFirst, int isLast, int isMoreThanOne);

void
printCSV(MatchingInfo *info, StreamContext* sc, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne);

void
printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse);

void
printCoarseSigList(CoarseSignature *list);

void
printStreamContext(StreamContext *sc);

#endif
