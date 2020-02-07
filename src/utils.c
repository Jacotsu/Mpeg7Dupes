#include "utils.h"



int
initFileIterator(struct fileIndex *fileIndex, char *fileListName) {
    Assert(fileIndex);
    FILE *listFile = fopen(fileListName, "r");
    Assert(listFile);
    fileIndex->indexA = 0;
    fileIndex->indexB = 1;
    fileIndex->maxIndex = getNumberOfLinesFromFilename(fileListName);

    // Max path length, 320 chars should be enough for most cases
    // To save more memory the paths could be allocated on request
    // 10 000 file paths should use 3 200 000 chars (3.2 kb)
    char* pathsMatrix = (char*) malloc(fileIndex->maxIndex*MAX_PATH_LENGTH);

    for (unsigned int i = 0; i < fileIndex->maxIndex; ++i) {
        fgets(&pathsMatrix[MAX_PATH_LENGTH*i], MAX_PATH_LENGTH, listFile);
        strtok(&pathsMatrix[MAX_PATH_LENGTH*i], "\n");
    }

    fileIndex->pathsMatrix = pathsMatrix;
    fclose(listFile);
    Assert(fileIndex->maxIndex);
    return 1;
}

unsigned int
getNumberOfLinesFromFilename(char *filename) {
    Assert(filename);
    FILE *listFile = fopen(filename, "r");
    unsigned int numbOfEntries = 0;
    char chr = 0;
    Assert(listFile);

    while (!feof(listFile)) {
        chr = getc(listFile);
        if (chr == '\n')
            ++numbOfEntries;
    }
    fclose(listFile);
    return numbOfEntries;
}

int
terminateFileIterator(struct fileIndex *fileIndex) {
    free(fileIndex->pathsMatrix);
    return 1;
}

// Given a fileIndex struct
// This function iterates over all the combinations of the lines
// in the file list
int
nextFileIteration(struct fileIndex *fileIndex, char *destBuffer,
    char *destBuffer2) {

    if (fileIndex->indexA >= fileIndex->maxIndex) {
        return 0;
    }

    if (fileIndex->indexB >= fileIndex->maxIndex) {
        ++fileIndex->indexA;
        fileIndex->indexB = fileIndex->indexA + 1;
        return nextFileIteration(fileIndex, destBuffer, destBuffer2);
    } else {
        strcpy(destBuffer, &fileIndex->pathsMatrix[fileIndex->indexA*\
            MAX_PATH_LENGTH]);
        strcpy(destBuffer2, &fileIndex->pathsMatrix[fileIndex->indexB*\
            MAX_PATH_LENGTH]);
        fileIndex->indexB++;
    }
    return 1;
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
