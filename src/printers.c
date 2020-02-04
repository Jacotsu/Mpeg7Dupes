#include "printers.h"

void printBeautifulHeader () {
    char strBuffer[170] = { 0 };
    printf("%46.46s %46.46s %9.9s %9.9s %11.11s %s\n",
        padStr("First signature", strBuffer, 40, ' '),
        padStr("Second signature",  &strBuffer[40], 51, ' '),
        padStr("score",  &strBuffer[91], 9, ' '),
        padStr("offset", &strBuffer[100], 9, ' '),
        "matchframes",
        "whole");
}

void
printBeautiful(MatchingInfo *info, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne) {

    if (isFirst) {
        if (isMoreThanOne) {
            printf("%-46.46s \u2533 %-46.46s %7d %7d %11d %1d\n",
                    file1, file2,
                    info->score, info->offset,\
                    info->matchframes, info->whole);
        } else {
            printf("%-46.46s \u2501 %-46.46s %7d %7d %11d %1d\n",
                    file1, file2,
                    info->score, info->offset,\
                    info->matchframes, info->whole);
        }
    } else if (isLast) {
        printf("%-46.46s \u2517 %-46.46s %7d %7d %11d %1d\n",
                " ", file2,
                info->score, info->offset,\
                info->matchframes, info->whole);
    } else {
        printf("%-46.46s \u2523 %-46.46s %7d %7d %11d %1d\n",
                " ", file2,
                info->score, info->offset,\
                info->matchframes, info->whole);
    }
}

void printCSVHeader () {
    printf("%s,%s,%s,%s,%s,%s\n", "First signature", "Second signature",\
        "score", "offset", "matchframes","whole");
}

void
printCSV(MatchingInfo *info, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne) {
    printf("%s,%s,%d,%d,%d,%d\n",
            file1, file2,
            info->score, info->offset,\
            info->matchframes, info->whole);
}
