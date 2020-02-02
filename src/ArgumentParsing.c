#include "ArgumentParsing.h"



static struct entry dict[] = {
    {"binary", BINARY},
    {"xml", XML},
    {"fast", MODE_FAST},
    {"full", MODE_FULL},
};


int
number_for_key(char *key)
{
    int i = 0;
    char *name = dict[i].str;
    while (name) {
        if (strcmp(name, key) == 0)
            return dict[i].n;
        name = dict[++i].str;
    }
    return 0;
}



static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    slog_debug(6, "%c %s", key, arg);
    switch (key) {
    case 'v': ++arguments->verbose; break;
    case 'm': arguments->mode  = number_for_key(arg); break;
    case 't': arguments->sigType  = number_for_key(arg);
              break;
    case 'd': printf("%s", arg);arguments->thD  = atof(arg); break;
    case 'c': arguments->thDc  = atof(arg); break;
    case 'x': arguments->thXh  = atof(arg); break;
    case 'i': arguments->thDi  = atof(arg); break;
    case 'b': arguments->thIt  = atof(arg); break;
    case ARGP_KEY_ARG: printf("%s",arg); return 0;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}



struct arguments
parseArguments(int argc, char **argv) {
    struct arguments arguments;
    const char *argp_program_version = "mpeg7Dupes 0.0.1";
    const char *argp_program_bug_address = "<dcdrj.pub@gmail.com>";
    static char doc[] = "Compare binary MPEG7 signatures to find visually"\
        " similar videos";
    static char args_doc[] = "[FILENAMES]...";
    static struct argp_option options[] = {
        { "verbosity", 'v', 0, 0, "Increase output verbosity"},
        { "lookup_mode", 'm', 0, 0, "Lookup mode: fast or full"},
        { "signature_type", 't', 0, 0, "Only binary is supported"},
        { "thD", 'd', 0, 0, "Threshold to detect one word as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 9000."},
        { "thDc", 'c', 0, 0, "Threshold to detect all words as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 60000."},
        { "thXh", 'x', 0, 0, "Threshold to detect all words as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 60000."},
        { "thDi", 'i', 0, 0, "The minimum length of a sequence in frames to "\
            "recognize it as matching sequence. The option value must be a "\
            "non negative integer value. The default value is 0."},
        { "thIt", 'b', 0, 0, "The minimum relation, that matching frames "\
            "to all frames must have. The option value must be a double "\
            "value between 0 and 1. The default value is 0.5."},
        { 0 }
    };

    arguments.verbose = 0;
    arguments.mode = MODE_FULL;
    arguments.sigType = BINARY;
    arguments.thD = 9000;
    arguments.thXh = 60000;
    arguments.thDi = 0;
    arguments.thIt = 0.5;
    arguments.numberOfPaths = 0;
    arguments.filePaths = NULL;


    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    // Bound checking and sanitization
    LoggedAssert(arguments.mode == FULL || arguments.mode == FAST,
        "Unsupported mode");
    LoggedAssert(arguments.sigType == BINARY, "Only binary signatures are"\
            "supported");
    LoggedAssert(arguments.thD > 0, "Word threshold must be positive");
    LoggedAssert(arguments.thdc > 0, "Detect threshold must be positive")
    LoggedAssert(arguments.thxH > 0, "Cumulative words threshold must be "\
        "positive");
    LoggedAssert(arguments.thDi > 0, "Minimum sequence length must be "\
        "positive");
    LoggedAssert(arguments.thIt > 0, "Minimum relation must be between 0 "\
        "and 1");

    for (unsigned int i = 0; i < arguments.numberOfPaths; ++i) {
        FILE *tmp = fopen(arguments.filePaths[i], "rb");
        LoggedAssert(tmp, "File %s not found, aborting");
        fclose(tmp);
    }

    return arguments;
};
