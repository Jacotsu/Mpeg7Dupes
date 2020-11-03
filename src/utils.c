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


int
xml_dump(StreamContext *sc)
{
    FILE* f;
    unsigned int pot3[5] = { 3*3*3*3, 3*3*3, 3*3, 3, 1 };

    // stdout
    f = stdout;

    /* header */
    fprintf(f, "<?xml version='1.0' encoding='ASCII' ?>\n");
    fprintf(f, "<Mpeg7 xmlns=\"urn:mpeg:mpeg7:schema:2001\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:mpeg:mpeg7:schema:2001 schema/Mpeg7-2001.xsd\">\n");
    fprintf(f, "  <DescriptionUnit xsi:type=\"DescriptorCollectionType\">\n");
    fprintf(f, "    <Descriptor xsi:type=\"VideoSignatureType\">\n");
    fprintf(f, "      <VideoSignatureRegion>\n");
    fprintf(f, "        <VideoSignatureSpatialRegion>\n");
    fprintf(f, "          <Pixel>0 0 </Pixel>\n");
    fprintf(f, "          <Pixel>%d %d </Pixel>\n", sc->w - 1, sc->h - 1);
    fprintf(f, "        </VideoSignatureSpatialRegion>\n");
    fprintf(f, "        <StartFrameOfSpatialRegion>0</StartFrameOfSpatialRegion>\n");
    /* hoping num is 1, other values are vague */
    fprintf(f, "        <MediaTimeUnit>%d</MediaTimeUnit>\n", sc->time_base.den / sc->time_base.num);
    fprintf(f, "        <MediaTimeOfSpatialRegion>\n");
    fprintf(f, "          <StartMediaTimeOfSpatialRegion>0</StartMediaTimeOfSpatialRegion>\n");
    fprintf(f, "          <EndMediaTimeOfSpatialRegion>%" PRIu64 "</EndMediaTimeOfSpatialRegion>\n", sc->coarseend->last->pts);
    fprintf(f, "        </MediaTimeOfSpatialRegion>\n");

    /* coarsesignatures */
    for (CoarseSignature* cs = sc->coarsesiglist; cs; cs = cs->next) {
        fprintf(f, "        <VSVideoSegment>\n");
        fprintf(f, "          <StartFrameOfSegment>%" PRIu32 "</StartFrameOfSegment>\n", cs->first->index);
        fprintf(f, "          <EndFrameOfSegment>%" PRIu32 "</EndFrameOfSegment>\n", cs->last->index);
        fprintf(f, "          <MediaTimeOfSegment>\n");
        fprintf(f, "            <StartMediaTimeOfSegment>%" PRIu64 "</StartMediaTimeOfSegment>\n", cs->first->pts);
        fprintf(f, "            <EndMediaTimeOfSegment>%" PRIu64 "</EndMediaTimeOfSegment>\n", cs->last->pts);
        fprintf(f, "          </MediaTimeOfSegment>\n");
        for (int i = 0; i < 5; i++) {
            fprintf(f, "          <BagOfWords>");
            for (int j = 0; j < 31; j++) {
                uint8_t n = cs->data[i][j];
                if (j < 30) {
                    fprintf(f, "%d  %d  %d  %d  %d  %d  %d  %d  ", (n & 0x80) >> 7,
                                                                   (n & 0x40) >> 6,
                                                                   (n & 0x20) >> 5,
                                                                   (n & 0x10) >> 4,
                                                                   (n & 0x08) >> 3,
                                                                   (n & 0x04) >> 2,
                                                                   (n & 0x02) >> 1,
                                                                   (n & 0x01));
                } else {
                    /* print only 3 bit in last byte */
                    fprintf(f, "%d  %d  %d ", (n & 0x80) >> 7,
                                              (n & 0x40) >> 6,
                                              (n & 0x20) >> 5);
                }
            }
            fprintf(f, "</BagOfWords>\n");
        }
        fprintf(f, "        </VSVideoSegment>\n");
    }

    /* finesignatures */
    for (FineSignature* fs = sc->finesiglist; fs; fs = fs->next) {
        fprintf(f, "        <VideoFrame>\n");
        fprintf(f, "          <MediaTimeOfFrame>%" PRIu64 "</MediaTimeOfFrame>\n", fs->pts);
        /* confidence */
        fprintf(f, "          <FrameConfidence>%d</FrameConfidence>\n", fs->confidence);
        /* words */
        fprintf(f, "          <Word>");
        for (int i = 0; i < 5; i++) {
            fprintf(f, "%d ", fs->words[i]);
            if (i < 4) {
                fprintf(f, " ");
            }
        }
        fprintf(f, "</Word>\n");
        /* framesignature */
        fprintf(f, "          <FrameSignature>");
        for (int i = 0; i< SIGELEM_SIZE/5; i++) {
            if (i > 0) {
                fprintf(f, " ");
            }
            fprintf(f, "%d ", fs->framesig[i] / pot3[0]);
            for (int j = 1; j < 5; j++)
                fprintf(f, " %d ", fs->framesig[i] % pot3[j-1] / pot3[j] );
        }
        fprintf(f, "</FrameSignature>\n");
        fprintf(f, "        </VideoFrame>\n");
    }
    fprintf(f, "      </VideoSignatureRegion>\n");
    fprintf(f, "    </Descriptor>\n");
    fprintf(f, "  </DescriptionUnit>\n");
    fprintf(f, "</Mpeg7>\n");
    fflush(f);

    return 0;
}
