#include "utils.h"

// Merges 2 fileIndex structs into one and returns a new fileIndex struct
// where the default new indexA and indexB are taken from the first
// struct and the default maxIndexes are the sum of the maxIndex of both
// structs
struct fileIndex
mergeFileIterators(struct fileIndex *index1, struct fileIndex *index2) {
    struct fileIndex newFileIndex = {0};
    char *newPathsMatrix = NULL;
    unsigned int maxFiles1 = FFMAX(index1->maxIndexA, index1->maxIndexB);
    unsigned int maxFiles2 = FFMAX(index2->maxIndexA, index2->maxIndexB);
    unsigned int maxFiles = maxFiles1 + maxFiles2;

    // Merge paths into memory
    newPathsMatrix = calloc(maxFiles, MAX_PATH_LENGTH);
    memcpy(newPathsMatrix, index1->pathsMatrix, maxFiles1*MAX_PATH_LENGTH);
    memcpy(newPathsMatrix + maxFiles1*MAX_PATH_LENGTH, index2->pathsMatrix,
        maxFiles2*MAX_PATH_LENGTH);

    newFileIndex.indexA = index1->indexA;
    newFileIndex.indexB = index1->indexB;
    newFileIndex.maxIndexA = maxFiles;
    newFileIndex.maxIndexB = maxFiles;
    newFileIndex.pathsMatrix = newPathsMatrix;

    return newFileIndex;
}

int
initFileIterator(struct fileIndex *fileIndex, char *fileListName) {
    Assert(fileIndex);
    FILE *listFile = fopen(fileListName, "r");
    Assert(listFile);
    fileIndex->indexA = -1;
    fileIndex->indexB = 0;
    fileIndex->maxIndexA = getNumberOfLinesFromFilename(fileListName);
    fileIndex->maxIndexB = fileIndex->maxIndexA;
    int maxFiles = FFMAX(fileIndex->maxIndexA, fileIndex->maxIndexB);


    // Max path length, 320 chars should be enough for most cases
    // To save more memory the paths could be allocated on request
    // 10 000 file paths should use 3 200 000 chars (3.2 kb)
    char* pathsMatrix = (char*) calloc(maxFiles, MAX_PATH_LENGTH);

    for (int i = 0; i < maxFiles; ++i) {
        fgets(&pathsMatrix[MAX_PATH_LENGTH*i], MAX_PATH_LENGTH, listFile);
        strtok(&pathsMatrix[MAX_PATH_LENGTH*i], "\n");
    }

    fileIndex->pathsMatrix = pathsMatrix;
    fclose(listFile);
    Assert(maxFiles);
    return 1;
}

int
initFileIteratorFromCmdLine(struct fileIndex *fileIndex,
	char **argv, int argc) {
    fileIndex->indexA = -1;
    fileIndex->indexB = 0;
    fileIndex->maxIndexA = argc;
    fileIndex->maxIndexB = argc;

    fileIndex->pathsMatrix = (char*) calloc(argc, MAX_PATH_LENGTH);
    for (int i = 0; i < argc; ++i)
        strcpy(&fileIndex->pathsMatrix[i*MAX_PATH_LENGTH], argv[i]);
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


int
nextFileIterationByIndex(struct fileIndex *fileIndex, char indexSelector) {
    switch (indexSelector) {
        case 'a': {
            ++fileIndex->indexA;
            if (fileIndex->indexA >= fileIndex->maxIndexA)
                return 0;
            break;
        }
        case 'b': {
            ++fileIndex->indexB;
            if (fileIndex->indexB >= fileIndex->maxIndexB) {
                fileIndex->indexB = fileIndex->indexA + 1;
                return 0;
            }
            break;
        }
    }
    return 1;
}

// Given a fileIndex struct
// This function iterates over all the combinations of the lines
// in the file list
int
nextFileIteration(struct fileIndex *fileIndex) {
    fileIndex->indexB++;

    if (fileIndex->indexA >= fileIndex->maxIndexA) {
        return 0;
    }

    if (fileIndex->indexB >= fileIndex->maxIndexB) {
        ++fileIndex->indexA;
        fileIndex->indexB = fileIndex->indexA;
        return nextFileIteration(fileIndex);
    }
    return 1;
}

char *
getIteratorIndexFilePath(struct fileIndex *fileIndex, char indexSelector){
    int effectiveIndex = 0;
    LoggedAssert(fileIndex->indexA >= 0 && fileIndex->indexB >= 0,
        "File iterator next function not called!");
    switch (indexSelector) {
        case 'a': effectiveIndex = fileIndex->indexA*MAX_PATH_LENGTH;
            break;
        case 'b': effectiveIndex = fileIndex->indexB*MAX_PATH_LENGTH;
            break;
    }
    return &fileIndex->pathsMatrix[effectiveIndex];
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


unsigned int
getFileSize(const char *filename) {
    int fileLength = 0;
    FILE *f = NULL;
    f = fopen(filename, "rb");
    LoggedAssert(f, "Can't open %s", filename);
    fseek(f, 0, SEEK_END);
    fileLength = ftell(f);
    fclose(f);
    return fileLength;
};


unsigned int
getPathLastSlashPosition(const char *path) {
    unsigned int pathLen = strlen(path);
    unsigned int lastSlashPosition = 0;

    for (unsigned int i = 0; i < pathLen; ++i) {
        if (i > 0) {
            if (path[i] == '/' && path[i - 1] != '/')
                lastSlashPosition = i;
        } else {
            if (path[i] == '/')
                lastSlashPosition = i;
        }
    }
    return lastSlashPosition;
}


unsigned int
buildDirectoryTree(const char *path) {
    char treePath[MAX_PATH_LENGTH] = {0};
    unsigned int pathStrLen = strlen(path);

    for (unsigned int j = 0; j < pathStrLen; ++j)
        if (path[j] == '/') {
            if (j != pathStrLen - 1) {
                memcpy(treePath, path, j);
                int status = mkdir(treePath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                LoggedAssert(status == 0, "Could not create path: %s", path);
            }
        }
    return 1;
}
