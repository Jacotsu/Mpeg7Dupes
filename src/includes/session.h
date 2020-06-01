#ifndef SESSION
#define SESSION

#include <stdio.h>
#include <stdlib.h>

#include "customAssert.h"
#include "ArgumentParsing.h"
#include "utils.h"

struct session {
    struct arguments *args;
    struct fileIndex *index;
};


int
saveSessionPrompt(struct session*);

int
initSession(struct session*, struct arguments*, struct fileIndex*);

int
saveSession(struct session*, char *destPath);

int
loadSession(struct arguments*, struct fileIndex*, char *sourcePath);

void
deleteSession(char *sessionPath);

#endif
