#include "main.h"

struct arguments args;

int
main(int argc, char **argv) {
    MatchingInfo result = {0};
    StreamContext scontexts[NUM_OF_INPUTS] = { 0 };

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

    char strBuffer[170] = { 0 };

    printf("%46.46s %46.46s %s %s %s %s\n",
        padStr("First signature", strBuffer, 40, ' '),
        padStr("Second signature",  &strBuffer[40], 51, ' '),
        padStr("score",  &strBuffer[91], 7, ' '),
        "offset",
        "matchframes",
        "whole");



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
                    printf("%-46.46s \u2533 %-46.46s %3d  %5d   %8d   %1d\n",
                        args.filePaths[i], args.filePaths[j],
                        result.score, result.offset,\
                        result.matchframes, result.whole);
                } else {
                    printf("%-46.46s \u2501 %-46.46s %3d  %5d   %8d   %1d\n",
                        args.filePaths[i], args.filePaths[j],
                        result.score, result.offset,\
                        result.matchframes, result.whole);
                }
            } else if (j == args.numberOfPaths - 1) {
                printf("%-46.46s \u2517 %-46.46s %3d  %5d   %8d   %1d\n",
                    " ", args.filePaths[j],
                    result.score, result.offset,\
                    result.matchframes, result.whole);
            } else {
                printf("%-46.46s \u2523 %-46.46s %3d  %5d   %8d   %1d\n",
                    " ", args.filePaths[j],
                    result.score, result.offset,\
                    result.matchframes, result.whole);
            }
        }
    }
    slog_info(4, "Signature processing finished");


    return 0;
}
