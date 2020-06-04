#include "session.h"

int
saveSessionPrompt(struct session* sess) {
    char sessionFilePath[MAX_PATH_LENGTH];
    printf("Please insert session destination path: ");
    scanf("%s", sessionFilePath);
    // safer for overflows but buggy
    //fgets(sessionFilePath , MAX_PATH_LENGTH , stdin);
    strtok(sessionFilePath, "\n");
    saveSession(sess, sessionFilePath);
    return 1;
}

int
initSession(struct session* sess, struct arguments* args,
    struct fileIndex* index) {
    sess->args = args;
    sess->index = index;
    return 1;
}

int
saveSession(struct session* sess, char *destPath) {
    FILE *sessionFile = NULL;
    unsigned int maxFiles = FFMAX(sess->index->maxIndexA, \
        sess->index->maxIndexB);

    buildDirectoryTree(destPath);
    sessionFile = fopen(destPath, "wb");
    LoggedAssert(sessionFile, "Cannot create session file: %s", destPath);

    fwrite(sess->args, sizeof(struct arguments), 1, sessionFile);
    fwrite(sess->index, sizeof(struct fileIndex), 1, sessionFile);
    fwrite(sess->index->pathsMatrix, sizeof(char)*MAX_PATH_LENGTH,
        maxFiles, sessionFile);

    fclose(sessionFile);
    slog_info(4, "Saved session file: %s", destPath);
    return 1;
}

int
loadSession(struct arguments* args, struct fileIndex* index,
    char *sourcePath) {

    FILE *sessionFile = NULL;
    unsigned int maxFiles = 0;

    sessionFile = fopen(sourcePath, "rb");
    LoggedAssert(sessionFile, "Session file not found : %s", sourcePath);

    fread(args, sizeof(struct arguments), 1, sessionFile);
    fread(index, sizeof(struct fileIndex), 1, sessionFile);
    maxFiles = FFMAX(index->maxIndexA, index->maxIndexB);
    index->pathsMatrix = calloc(sizeof(char), maxFiles*MAX_PATH_LENGTH);
    fread(index->pathsMatrix, sizeof(char)*MAX_PATH_LENGTH, maxFiles,
        sessionFile);

    fclose(sessionFile);
    slog_info(4, "Loaded session file: %s", sourcePath);
    return 1;
}

void
deleteSession(char *sessionPath){
    if(!remove(sessionPath))
        slog_error(2, "Error deleting file")
    else
        slog_info(4, "File successfully deleted");
}
