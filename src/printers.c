#include "printers.h"

void
printBeautifulHeader () {
    char strBuffer[170] = { 0 };
    #pragma omp ordered
    printf("%46.46s %46.46s %9.9s %12.12s %12.12s %5.5s\n",
        padStr("First signature", strBuffer, 40, ' '),
        padStr("Second signature",  &strBuffer[40], 51, ' '),
        padStr("score",  &strBuffer[91], 9, ' '),
        padStr("time 1 [s]", &strBuffer[100], 12, ' '),
        padStr("time 2 [s]", &strBuffer[112], 12, ' '),
        "whole");
}

void
printBeautiful(MatchingInfo *info, StreamContext* sc, char *file1,\
    char *file2, int isFirst, int isLast, int isMoreThanOne) {

    unsigned int selectedFormatStr = 0;
    char *firstFilePath = file1;
    char *formatStrings[] = {
        // First more than one
        "%-46.46s \u2533 %-46.46s %7d %12.2f %12.2f %1d\n",
        "%-46.46s \u2501 %-46.46s %7d %12.2f %12.2f %1d\n",
        // last
        "%-46.46s \u2517 %-46.46s %7d %12.2f %12.2f %1d\n",
        "%-46.46s \u2523 %-46.46s %7d %12.2f %12.2f %1d\n"
    };

    #pragma omp ordered
    if (info->score) {
        if (isFirst) {
            if (isMoreThanOne) {
                selectedFormatStr = 0;
            } else {
                selectedFormatStr = 1;
            }
        } else if (isLast) {
            // We don't want to always print the first file path
            firstFilePath = " ";
            selectedFormatStr = 2;
        } else {
            firstFilePath = " ";
            selectedFormatStr = 3;
        }

        printf(formatStrings[selectedFormatStr], firstFilePath, file2, info->score,\
                ((double) info->first->pts * sc[0].time_base.num) / sc[0].time_base.den,
                ((double) info->second->pts * sc[1].time_base.num) / sc[1].time_base.den,
                info->whole);
    }
}

// TO update with new offsets
void
printCSVHeader () {
    #pragma omp ordered
    printf("%s,%s,%s,%s,%s,%s\n", "First signature", "Second signature",\
        "score", "time 1 [s]", "time 2 [s]", "whole");
}

void
printCSV(MatchingInfo *info, StreamContext* sc, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne) {
    #pragma omp ordered
    if (info->score)
        printf("%s,%s,%d,%.2f,%.2f,%d\n",
                file1, file2,
                info->score,
                // pts is the frame number
                ((double) info->first->pts * sc[0].time_base.num) / sc[0].time_base.den,
                ((double) info->second->pts * sc[1].time_base.num) / sc[1].time_base.den,\
                info->whole);
}

void
printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse) {
    #pragma omp ordered
    for (FineSignature *i = list; i != end && i; i = i->next) {
        if (lastCoarse) {
            if (i->next != end) {
                slog_debug(6,"  \u2523\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"  \u2503 \u2517\u2501 Pts: %lu\t"\
                    "Confidence: %hhu", i->pts, i->confidence);
            } else {
                slog_debug(6,"  \u2517\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"    \u2517\u2501 Pts: %lu\t"\
                    "Confidence: %hhu", i->pts, i->confidence);
            }
        } else {
            if (i->next != end) {
                slog_debug(6,"\u2503 \u2523\u2501\u2533 Fine signature at %p",\
                        i);
                slog_debug(6,"\u2503 \u2503 \u2517\u2501 Pts: %lu\t\t"\
                    "Confidence: %hhu", i->pts, i->confidence);
            } else {
                slog_debug(6,"\u2503 \u2517\u2501\u2533 Fine signature at %p",\
                        i);
                slog_debug(6,"\u2503   \u2517\u2501 Pts: %lu\t"\
                    "Confidence: %hhu", i->pts, i->confidence);
            }
        }
    }
}

void
printCoarseSigList(CoarseSignature *list) {
    #pragma omp ordered
    for (CoarseSignature *j = list; j->next ; j = j->next) {
        if (j->next->next) {
            slog_debug(6,"\u2523\u2533 Coarse signature at %p", j);
            slog_debug(6,"\u2503\u2523\u2501\u2578 Coarse signature bounds: "\
                "%lu %lu", j->first ? j->first->pts : -1,\
                j->last ? j->last->pts : -1);
            if (j->first && j->first->next) {
                slog_debug(6,"\u2503\u2517\u2533\u2578 Fine signatures "\
                    "bounds: %p %p", j->first, j->last);
            } else {
                slog_debug(6,"\u2503\u2517\u2501\u2578 Fine signatures "\
                    "bounds: %p %p", j->first, j->last);
            }
            printFineSigList(j->first, j->last, 0);
        } else {
            slog_debug(6,"\u2517\u2533 Coarse signature at %p", j);
            slog_debug(6," \u2523\u2501\u2578 Coarse signature bounds: "\
                "%lu %lu", j->first ? j->first->pts : -1,\
                j->last ? j->last->pts : -1);
            slog_debug(6," \u2517\u2501\u2578 Fine signatures "\
                "bounds: %p %p", list->first, list->last);
            printFineSigList(j->first, j->last, 1);
        }
    }
}

void
printStreamContext(StreamContext* sc) {
    #pragma omp ordered
    {
        slog_debug(6,"\u250F Time base: %d/%d", sc->time_base.num,\
                sc->time_base.den);
        slog_debug(6,"\u2523 Width: %d\tHeight: %d",sc->w,sc->h);
        slog_debug(6,"\u2523 Overflow protection: %d", sc->divide);
        slog_debug(6,"\u2523 Fine signatures list: %p", sc->finesiglist);
        slog_debug(6,"\u2523 Coarse signature list: %p", sc->coarsesiglist);
        slog_debug(6,"\u2523 Last index: %d", sc->lastindex);
        slog_debug(6,"\u2523 Signatures:", sc->lastindex);
        printCoarseSigList(sc->coarsesiglist);
    }
}


void
printResult(
    struct fileIndex *index,
    MatchingInfo *result,
    SignatureContext *sigContext,
    int minimumScore,
    void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne)) {

    // These are necessary because otherwhise the correct format string
    // is used only when the first printed value is actually the first
    // file in the list
    static int hasFirstBeenPrinted = 0;


    #pragma omp ordered
    {
        Assert(index);
        Assert(result);
        Assert(sigContext);
        Assert(printFunctionPointer);

        if (result->score >= minimumScore) {
            int i = index->indexA;
            int j = index->indexB;
            StreamContext *scontexts = sigContext->streamcontexts;
            char *filePath1 = &index->pathsMatrix[i*MAX_PATH_LENGTH];
            char *filePath2 = &index->pathsMatrix[j*MAX_PATH_LENGTH];

            if (j == i + 1) {
                // Print first element
                if (index->maxIndex - j > 1) {
                    printFunctionPointer(result, scontexts, filePath1, filePath2,
                            1, 0, 1);
                    hasFirstBeenPrinted = 1;
                } else {
                    printFunctionPointer(result, scontexts, filePath1, filePath2,
                            1, 0, 0);
                }
                hasFirstBeenPrinted = 1;
            } else if (j == index->maxIndex - 1) {
                // Print last element
                printFunctionPointer(result, scontexts, filePath1, filePath2,
                        0, 1, 0);
            } else {
                // Print intermediate element
                if (hasFirstBeenPrinted)
                    printFunctionPointer(result, scontexts, filePath1, filePath2,
                            0, 0, 1);
                else {
                    if (index->maxIndex - j > 1) {
                        printFunctionPointer(result, scontexts, filePath1, filePath2,
                                1, 0, 1);
                        hasFirstBeenPrinted = 1;
                    } else {
                        printFunctionPointer(result, scontexts, filePath1, filePath2,
                                1, 0, 0);
                    }
                    hasFirstBeenPrinted = 1;
                }
            }

            fflush(stdout);
        }
    }
}
