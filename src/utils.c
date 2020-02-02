#include "utils.h"

void
printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse) {
    for (FineSignature *i = list; i != end; i = i->next) {
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
                "%lu %lu", j->first->pts, j->last->pts);
            slog_debug(6,"\u2503\u2517\u2533\u2578 Fine signatures "\
                "bounds: %p %p", list->first, list->last);
            printFineSigList(list->first, list->last, 0);
        } else {
            slog_debug(6,"\u2517\u2533 Coarse signature at %p", j);
            slog_debug(6," \u2523\u2501\u2578 Coarse signature bounds: "\
                "%lu %lu", j->first->pts, j->last->pts);
            slog_debug(6," \u2517\u2533\u2578 Fine signatures "\
                "bounds: %p %p", list->first, list->last);
            printFineSigList(list->first, list->last, 1);
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

int
fineSignatureCmp(const void* p1, const void* p2) {
    FineSignature *a = (FineSignature*) p1;
    FineSignature *b = (FineSignature*) p2;
    if (a->pts == b->pts)
        return 0;
    else if (a->pts < b->pts)
        return -1;
    else
        return 1;
};
