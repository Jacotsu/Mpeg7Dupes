#include "main.h"
#include "utils.h"

int
main(int argc, char **argv) {
    StreamContext *a, *b;
    MatchingInfo result = {0};
	struct arguments options;
    // 0    panic
    // 1    fatal
    // 2    error
    // 3    warn
    // 4    info
    // 5    live
    // 6    debug
    if (__DEBUG)
        slog_init("logfile", "slog.cfg", 6, 1);
    else
        slog_init("logfile", "slog.cfg", 5, 1);
	options = parseArguments(argc, argv);


	SignatureContext sigContext = {
        .class = NULL,
        .mode = MODE_FULL,
        .nb_inputs = 1,
        .filename = "",
        .thworddist = 9000,
        .thcomposdist = 60000,
        .thl1 = 116,
        .thdi = 0,
        .thit = 1
    };


    a = binary_import("1234_In_the_name_of_GodCCS_tarrant.webm.sig");
    b = binary_import("1234_In_the_name_of_GodCCS_tarrant.webm.sig");
    printStreamContext(a);
    //c = binary_import("_0.webm.sig");

    //static MatchingInfo
	//lookup_signatures(
    //SignatureContext *sc,
    //StreamContext *first,
    //StreamContext *second,
    //int mode)
    //result = lookup_signatures(signatureContext, streaContext1, streamContext2, mode);
    result = lookup_signatures(&sigContext, a, b, sigContext.mode);
    slog_info(4, "score: %d offset: %d matchframes: %d whole: %d",\
            result.score, result.offset, result.matchframes, result.whole);

    //result = lookup_signatures(&sigContext, a, c, sigContext.mode);
    //slog_info(4, "score: %d offset: %d matchframes: %d whole: %d",\
    //        result.score, result.offset, result.matchframes, result.whole);

    return 0;
}
