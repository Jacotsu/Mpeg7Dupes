#include "utils.h"

int
initFileIterator(struct fileIndex *fileIndex, char *fileListName) {
    Assert(fileIndex);
    FILE *listFile = fopen(fileListName, "r");
    Assert(listFile);
    fileIndex->indexA = 0;
    fileIndex->indexB = 0;
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

    fileIndex->indexB++;

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
