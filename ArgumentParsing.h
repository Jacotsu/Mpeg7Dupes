#ifndef ARGUMENT_PARSING
#define ARGUMENT_PARSING


#include <argp.h>
#include <string.h>
#include <stdlib.h>

#include "signature.h"

enum signatureType {
	BINARY, XML
};

struct arguments {
    enum lookup_mode mode;
    enum signatureType sigType;
	double thD, thDc, thXh, thDi, thIt;
};


struct arguments parseArguments(int, char**);

#endif
