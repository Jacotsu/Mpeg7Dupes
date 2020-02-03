#include "ArgumentParsing.h"

int
numberForKey(char *key)
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
    // Key and param seems to be mutually exclusive, so we use this flag
    // to remember the last used key so that we can assign the correct
    // value
    static char lastKeyFlag = 0;
    static unsigned int numUsedArgs = 0;
    switch (key) {
    case 'v': lastKeyFlag = 'v'; break;
    case 'm': lastKeyFlag = 'm'; break;
    case 't': lastKeyFlag = 't'; break;
    case 'd': lastKeyFlag = 'd'; break;
    case 'c': lastKeyFlag = 'c'; break;
    case 'x': lastKeyFlag = 'x'; break;
    case 'i': lastKeyFlag = 'i'; break;
    case 'b': lastKeyFlag = 'b'; break;
    case ARGP_KEY_INIT:
        slog_debug(6, "Initializing arg parsing");
        arguments->verbose = 0;
        arguments->mode = MODE_FULL;
        arguments->sigType = BINARY;
        arguments->thD = 9000;
        arguments->thXh = 60000;
        arguments->thDi = 0;
        arguments->thIt = 0.5;
        arguments->numberOfPaths = 0;
        arguments->filePaths = NULL;
        break;

    case ARGP_KEY_ARG:
        if (lastKeyFlag)
            ++numUsedArgs;

        switch (lastKeyFlag) {
            case 'v': ++arguments->verbose; break;
            case 'm': if (arg) arguments->mode  = numberForKey(arg); break;
            case 't': if (arg) arguments->sigType  = numberForKey(arg); break;
            case 'd': if (arg) arguments->thD  = atof(arg); break;
            case 'c': if (arg) arguments->thDc  = atof(arg); break;
            case 'x': if (arg) arguments->thXh  = atof(arg); break;
            case 'i': if (arg) arguments->thDi  = atof(arg); break;
            case 'b': if (arg) arguments->thIt  = atof(arg); break;
        }
        // Remember to reset the keyflag
        lastKeyFlag = 0;
        break;
    case ARGP_KEY_END:
        if (state->arg_num < 2) {
            slog_error(2, "You should supply at least 2 files");
            argp_usage(state);
        } else {
            // First path is executable
            arguments->numberOfPaths = state->arg_num - numUsedArgs;
            arguments->filePaths = state->argv + state->argc -\
                arguments->numberOfPaths;
            // Bound checking and sanitization
            LoggedAssert(arguments->mode == MODE_FULL ||\
                arguments->mode == MODE_FAST,
                "Unsupported mode");
            LoggedAssert(arguments->sigType == BINARY,
                "Only binary signatures are supported");
            LoggedAssert(arguments->thD >= 0,\
                "Word threshold must be positive");
            LoggedAssert(arguments->thDc >= 0,\
                "Detect threshold must be positive")
            LoggedAssert(arguments->thXh >= 0,\
                "Cumulative words threshold must be positive");
            LoggedAssert(arguments->thDi >= 0,\
                "Minimum sequence length must be positive");
            LoggedAssert(arguments->thIt >= 0,\
                "Minimum relation must be between 0 and 1");

            for (unsigned int i = 0; i < arguments->numberOfPaths; ++i) {
                FILE *tmp = fopen(arguments->filePaths[i], "rb");
                LoggedAssert(tmp, "File %s not found, aborting");
                fclose(tmp);
            }
        }
        break;
    default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}



struct arguments
parseArguments(int argc, char **argv) {
    struct arguments arguments;
    error_t result;
    const char *argp_program_version = "mpeg7Dupes 0.0.1";
    const char *argp_program_bug_address = "<dcdrj.pub@gmail.com>";
    char doc[] = "Compare binary MPEG7 signatures to find visually"\
        " similar videos";
    char args_doc[] = "[FILE1] [FILE2] ...";
    struct argp_option options[] = {
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

    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
    result = argp_parse(&argp, argc, argv, 0, 0, &arguments);
    LoggedAssert(!result, "Argument parsing failed");

    return arguments;
};
