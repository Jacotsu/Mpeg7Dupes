#include "printers.h"

void
printBeautifulHeader () {
    char strBuffer[170] = { 0 };
    printf("%46.46s %46.46s %9.9s %10.10s %10.10s %s\n",
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
    printf("%s,%s,%s,%s,%s,%s\n", "First signature", "Second signature",\
        "score", "time 1 [s]", "time 2 [s]", "whole");
}

void
printCSV(MatchingInfo *info, StreamContext* sc, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne) {
    printf("%s,%s,%d,%f,%f,%d\n",
            file1, file2,
            info->score,
            // pts is the frame number
            ((double) info->first->pts * sc[0].time_base.num) / sc[0].time_base.den,
            ((double) info->second->pts * sc[1].time_base.num) / sc[1].time_base.den,\
            info->whole);
}

void
printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse) {
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
