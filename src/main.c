#include "main.h"

struct arguments args;

int
main(int argc, char **argv) {
    struct fileIndex index = {0};
    int useOpenMp = 0;
    void (*printFunctionPointer)(MatchingInfo *info, StreamContext* sc,\
        char *file1, char *file2, int isFirst, int isLast, int isMoreThanOne)\
        = printBeautiful;


    slog_init("logfile", "slog.cfg", 5, 1);
	args = parseArguments(argc, argv);

    slog_info(4, "Logging initialized");
    // 0    panic
    // 2    error
    // 3    warn
    // 4    info
    // 5    live
    // 6    debug
    if (__DEBUG || args.verbose) {
        slog_init("logfile", "slog.cfg", 6, 1);
    }


    if (args.outputFormat == CSV) {
        printCSVHeader();
        printFunctionPointer = printCSV;
        // In CSV files output order is not really important
        useOpenMp = 1;
    } else {
        printBeautifulHeader();
        printFunctionPointer = printBeautiful;
    }


    if (args.listFile) {
        initFileIterator(&index, args.listFile);
        processFiles(&index, printFunctionPointer, useOpenMp);
        terminateFileIterator(&index);
    } else {
        initFileIteratorFromCmdLine(&index, args.filePaths,\
            args.numberOfPaths);
        processFiles(&index, printFunctionPointer, useOpenMp);
        terminateFileIterator(&index);
    }
    slog_info(4, "Signature processing finished");

    return 0;
}


void
processFiles(struct fileIndex *index, void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne), int useOpenMp) {

    while(nextFileIterationByIndex(index, 'a')) {
        if (useOpenMp) {
            #pragma omp parallel
            while (nextFileIterationByIndex(index, 'b'))
                processFilePair(index, printFunctionPointer);
        } else {
            while (nextFileIterationByIndex(index, 'b'))
                processFilePair(index, printFunctionPointer);
        }
    }
}

void
processFilePair(struct fileIndex *index, void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne)) {
    StreamContext scontexts[NUM_OF_INPUTS] = { 0 };
    MatchingInfo result = {0};
    char *filePath1 = getIteratorIndexFilePath(index, 'a');
    char *filePath2 = getIteratorIndexFilePath(index, 'b');

    SignatureContext sigContext = {
        .class = NULL,
        .mode = args.mode,
        .nb_inputs = NUM_OF_INPUTS,
        .filename = "",
        .thworddist = args.thD,
        .thcomposdist = args.thDc,
        .thl1 = args.thXh,
        .thdi = args.thDi,
        .thit = args.thIt,
        .streamcontexts = scontexts
    };


    binary_import(&scontexts[0], filePath1);
    printStreamContext(&scontexts[0]);

    binary_import(&scontexts[1], filePath2);
    printStreamContext(&scontexts[1]);

    slog_debug(6, "Processing %s\t%s", filePath1, filePath2);

    result = lookup_signatures(&sigContext, &scontexts[0],\
            &scontexts[1], sigContext.mode);

    int i = index->indexA;
    int j = index->indexB;
    if (j == i + 1) {
        if (index->maxIndex - j > 1) {
            printFunctionPointer(&result, scontexts, filePath1, filePath2,
                    1, 0, 1);
        } else {
            printFunctionPointer(&result, scontexts, filePath1, filePath2,
                    1, 0, 0);
        }
    } else if (j == index->maxIndex - 1) {
        printFunctionPointer(&result, scontexts, filePath1, filePath2,
                0, 1, 0);
    } else {
        printFunctionPointer(&result, scontexts, filePath1, filePath2,
                0, 0, 1);
    }

    fflush(stdout);
    signature_unload(&scontexts[1]);
    signature_unload(&scontexts[0]);
}
