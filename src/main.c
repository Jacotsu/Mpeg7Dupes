#include "main.h"

struct arguments args;

int
main(int argc, char **argv) {
    MatchingInfo result = {0};

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
        .nb_inputs = 1,
        .filename = "",
        .thworddist = args.thD,
        .thcomposdist = args.thDc,
        .thl1 = args.thXh,
        .thdi = args.thDi,
        .thit = args.thIt
    };

    char strBuffer[170] = { 0 };
    /*printf("%s\t%s\t%s\t%s\t%s\t%s\n",
        padStr("First signature", strBuffer, 40, ' '),
        padStr("Second signature",  &strBuffer[40], 55, ' '),
        padStr("score", &strBuffer[95], 17, ' '),
        padStr("offset", &strBuffer[150], 7, ' '),
        padStr("matchframes", &strBuffer[153], 12, ' '),
        padStr("whole", &strBuffer[165], 6, ' '));*/

    printf("%s %s %s %s %s %s\n",
        padStr("First signature", strBuffer, 40, ' '),
        padStr("Second signature",  &strBuffer[40], 51, ' '),
        padStr("score",  &strBuffer[91], 7, ' '),
        "offset",
        "matchframes",
        "whole");



    for (unsigned int i = 0; i < args.numberOfPaths; ++i)
        for(unsigned int j = i + 1; j < args.numberOfPaths; ++j) {
            StreamContext *sig1 = binary_import(args.filePaths[i]);
            printStreamContext(sig1);
            StreamContext *sig2 = binary_import(args.filePaths[j]);
            printStreamContext(sig2);
            slog_debug(6, "Processing %s\t%s", args.filePaths[i], \
                args.filePaths[j]);

            result = lookup_signatures(&sigContext, sig1, sig2,
                sigContext.mode);

            printf("%28s %28s %3d  %5d   %8d   %1d\n",
                args.filePaths[i], args.filePaths[j],
                result.score, result.offset,\
                result.matchframes, result.whole);
        }
    slog_info(4, "Signature processing finished");


    return 0;
}
