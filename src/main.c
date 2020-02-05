#include "main.h"

struct arguments args;

int
main(int argc, char **argv) {
    MatchingInfo result = {0};
    StreamContext scontexts[NUM_OF_INPUTS] = { 0 };
    void (*printFunctionPointer)(MatchingInfo *info, char *file1, char *file2,\
    int isFirst, int isLast, int isMoreThanOne) = printBeautiful;
;

    slog_init("logfile", "slog.cfg", 5, 1);
	args = parseArguments(argc, argv);

    slog_info(4, "Logging initialized");
    // 0    panic
    // 1    fatal
    // 2    error
    // 3    warn
    // 4    info
    // 5    live
    // 6    debug
    if (__DEBUG || args.verbose) {
        slog_init("logfile", "slog.cfg", 6, 1);
    }

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

    if (args.outputFormat == CSV) {
        printCSVHeader();
        printFunctionPointer = printCSV;
    } else {
        printBeautifulHeader();
        printFunctionPointer = printBeautiful;
    }


    if (args.listFile) {
        struct fileIndex index;
        char file1[200], file2[200];
        initFileIterator(&index, args.listFile);

        while (nextFileIteration(&index, file1, file2, 200))
            printf("%s %s\n", file1, file2);

        terminateFileIterator(&index);
    } else {
        for (unsigned int i = 0; i < args.numberOfPaths; ++i) {
            binary_import(&scontexts[0], args.filePaths[i]);
            printStreamContext(&scontexts[0]);
            for(unsigned int j = i + 1; j < args.numberOfPaths; ++j) {
                binary_import(&scontexts[1], args.filePaths[j]);
                printStreamContext(&scontexts[1]);
                slog_debug(6, "Processing %s\t%s", args.filePaths[i], \
                    args.filePaths[j]);

                result = lookup_signatures(&sigContext, &scontexts[0],\
                    &scontexts[1], sigContext.mode);

                if (j == i + 1) {
                    if (args.numberOfPaths - j > 1) {
                        printFunctionPointer(&result, args.filePaths[i],
                            args.filePaths[j], 1, 0, 1);
                    } else {
                        printFunctionPointer(&result, args.filePaths[i],
                            args.filePaths[j], 1, 0, 0);
                    }
                } else if (j == args.numberOfPaths - 1) {
                    printFunctionPointer(&result, args.filePaths[i],
                        args.filePaths[j], 0, 1, 0);
                } else {
                    printFunctionPointer(&result, args.filePaths[i],
                        args.filePaths[j], 0, 0, 1);
                }
            }
        }
    }
    slog_info(4, "Signature processing finished");


    return 0;
}
