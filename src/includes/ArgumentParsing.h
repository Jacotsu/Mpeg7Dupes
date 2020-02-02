#ifndef ARGUMENT_PARSING
#define ARGUMENT_PARSING


#include <argp.h>
#include <string.h>
#include <stdlib.h>

#include "slog.h"
#include "customAssert.h"
#include "signature.h"

#include <string.h>



enum signatureType {
	BINARY, XML
};

struct arguments {
    int verbose;
    enum lookup_mode mode;
    enum signatureType sigType;
	double thD, thDc, thXh, thDi, thIt;
    char **filePaths;
    unsigned int numberOfPaths;
};

// https://stackoverflow.com/questions/6669842/how-to-best-achieve-string-
// to-number-mapping-in-a-c-program
struct entry {
    char *str;
    int n;
};


int number_for_key(char *key);

struct arguments parseArguments(int, char**);

#endif
