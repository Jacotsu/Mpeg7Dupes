#include "utils.h"



int
initFileIterator(struct fileIndex *fileIndex, char *fileListName) {
    Assert(fileIndex);
    fileIndex->fileList = fopen(fileListName, "r");
    LoggedAssert(fileIndex->fileList, "Failed to open %s", fileListName);
    fileIndex->indexA = ftell(fileIndex->fileList);
    fileIndex->indexB = ftell(fileIndex->fileList);

    return 1;
}

unsigned int
getNumberOfLinesFromIterator(struct fileIndex *fileIndex) {
    Assert(fileIndex->fileList);
    long currentIndex = ftell(fileIndex->fileList);
    unsigned int lineNumber = 0;
    char chr = EOF;

    do {
        chr = getc(fileIndex->fileList);
        if (chr == '\n')
            ++lineNumber;
    } while (chr != EOF);

    fseek(fileIndex->fileList, currentIndex, SEEK_SET);
    return lineNumber;
}

int
terminateFileIterator(struct fileIndex *fileIndex) {
    Assert(fileIndex);
    fclose(fileIndex->fileList);
    return 1;
}

// Given a fileIndex struct
// This function iterates over all the combinations of the lines
// in the file list
int
nextFileIteration(struct fileIndex *fileIndex,
    char *destBuffer, char *destBuffer2, int maxLen) {
    Assert(fileIndex);

    if (feof(fileIndex->fileList)) {
        fseek(fileIndex->fileList, fileIndex->indexA, SEEK_SET);
        fgets (destBuffer, maxLen, fileIndex->fileList);
        // Remove newlines from path
        strtok(destBuffer, "\n");
        if (!feof(fileIndex->fileList)) {
            fileIndex->indexA = ftell(fileIndex->fileList);
            fgets (destBuffer2, maxLen, fileIndex->fileList);
            strtok(destBuffer2, "\n");
            if (!feof(fileIndex->fileList)) {
                fileIndex->indexB = ftell(fileIndex->fileList);
                return 1;
            }
        }
    } else {
        fseek(fileIndex->fileList, fileIndex->indexA, SEEK_SET);
        fgets (destBuffer, maxLen, fileIndex->fileList);
        strtok(destBuffer, "\n");

        if (!feof(fileIndex->fileList)) {
            fseek(fileIndex->fileList, fileIndex->indexB, SEEK_SET);
            fgets (destBuffer2, maxLen, fileIndex->fileList);
            strtok(destBuffer2, "\n");
            fileIndex->indexB = ftell(fileIndex->fileList);
            return 1;
        }
    }
    return 0;
}

char*
padStr(char *str, char *buffer, int maxLen, char padChar) {
    int len, padSpace;
    Assert(str);
    Assert(buffer);

    len = strlen(str);
    Assert(len <= maxLen);

    padSpace = (maxLen - len)/2;
    Assert(padSpace >= 0);
    for (int i = 0;  i < padSpace;++i)
        buffer[i] = padChar;

    strcat(&buffer[padSpace], str);
    for (int i = len+padSpace; i < maxLen && i < maxLen;++i)
        buffer[i] = padChar;
    buffer[maxLen-1] = '\0';
    return buffer;
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
