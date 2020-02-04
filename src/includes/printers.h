#ifndef PRINTERS
#define PRINTERS

#include "signature.h"
#include "utils.h"

void
printBeautifulHeader();

void
printCSVHeader();

void
printBeautiful(MatchingInfo *info, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne);

void
printCSV(MatchingInfo *info, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne);

#endif
