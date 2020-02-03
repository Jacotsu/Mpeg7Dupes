#include "main.h"

struct arguments args;

int
main(int argc, char **argv) {
    MatchingInfo result = {0};

    slog_init("logfile", "slog.cfg", 5, 1);
	args = parseArguments(argc, argv);

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


    for (unsigned int i = 0; i < args.numberOfPaths; ++i)
        for(unsigned int j = i; j < args.numberOfPaths; ++j) {
            StreamContext *sig1 = binary_import(args.filePaths[i]);
            printStreamContext(sig1);
            StreamContext *sig2 = binary_import(args.filePaths[j]);
            printStreamContext(sig2);

            result = lookup_signatures(&sigContext, sig1, sig2,
                sigContext.mode);
            slog_info(4, "%20.20s\t%20.20s\t%5.5s\t%6.6s\t"\
                    "%11.11s\t%5.5s", "First signature", "Second signature",
                    "score","offset", "matchframes", "whole");
            slog_info(4, "%20s\t%20s\t%5d\t%6d\t%11d\t%5d", args.filePaths[i],\
                args.filePaths[j], result.score, result.offset,\
                result.matchframes, result.whole);
        }


    return 0;
}
