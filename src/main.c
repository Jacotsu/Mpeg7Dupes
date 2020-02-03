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
        .mode = MODE_FULL,
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
            slog_info(4, "score: %d offset: %d matchframes: %d whole: %d",\
                result.score, result.offset, result.matchframes, result.whole);
        }


    //static MatchingInfo
	//lookup_signatures(
    //SignatureContext *sc,
    //StreamContext *first,
    //StreamContext *second,
    //int mode)
    //result = lookup_signatures(signatureContext, streaContext1, streamContext2, mode);
    /*result = lookup_signatures(&sigContext, a, b, sigContext.mode);
    slog_info(4, "score: %d offset: %d matchframes: %d whole: %d",\
            result.score, result.offset, result.matchframes, result.whole);*/

    //result = lookup_signatures(&sigContext, a, c, sigContext.mode);
    //slog_info(4, "score: %d offset: %d matchframes: %d whole: %d",\
    //        result.score, result.offset, result.matchframes, result.whole);

    return 0;
}
