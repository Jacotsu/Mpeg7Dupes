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
    return -1;
}



static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;
    // Key and param seems to be mutually exclusive, so we use this flag
    // to remember the last used key so that we can assign the correct
    // value
    switch (key) {
        case 'v': ++arguments->verbose; break;
        case 'p': arguments->useOpenMp  = 1; break;
        case 'm': if (arg) arguments->mode  = numberForKey(arg); break;
        case 't': if (arg) arguments->sigType  = numberForKey(arg); break;
        case 'd': if (arg) arguments->thD  = atof(arg); break;
        case 'c': if (arg) arguments->thDc  = atof(arg); break;
        case 'x': if (arg) arguments->thXh  = atof(arg); break;
        case 'i': if (arg) arguments->thDi  = atof(arg); break;
        case 'b': if (arg) arguments->thIt  = atof(arg); break;
        case 'k': if (arg) arguments->minScore  = atof(arg); break;
        case 'f': if (arg) arguments->outputFormat  = numberForKey(arg);
                      break;
        case 'l': if (arg) arguments->listFile = arg; break;
        case 's': if (arg) arguments->sessionFile = arg; break;
        case 'n': if (arg) arguments->incrementalFile = arg; break;
        case ARGP_KEY_ARG:
            break;
        case ARGP_KEY_INIT:
            slog_debug(6, "Initializing arg parsing");
            arguments->verbose = 0;
            arguments->listFile = NULL;
            arguments->sessionFile = NULL;
            arguments->incrementalFile = NULL;
            arguments->mode = MODE_FAST;
            arguments->sigType = BINARY;
            arguments->outputFormat = BEAUTIFUL;
            arguments->thD = 9000;
            arguments->thDc = 60000;
            arguments->thXh = 290;
            arguments->thDi = 150;
            arguments->thIt = 0.5;
            arguments->numberOfPaths = 0;
            arguments->useOpenMp = 0;
            arguments->filePaths = NULL;
            arguments->minScore = 9000;
            break;

        case ARGP_KEY_END:
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
            LoggedAssert(arguments->minScore > 0,\
                "Minimum score must be >0");
            LoggedAssert(arguments->outputFormat == BEAUTIFUL ||
                arguments->outputFormat == CSV,\
                "Output format not supported");

            if (arguments->sessionFile) {
                AssertFileExistence(arguments->sessionFile,
                    "Session file not found");
            }


            if (arguments->incrementalFile) {
                AssertFileExistence(arguments->incrementalFile,
                    "Incremental file list not found");
            }

            if (state->arg_num < 2 && !arguments->listFile) {
                slog_error(2, "You should supply at least 2 files");
                argp_usage(state);
            } else if (arguments->listFile){
                AssertFileExistence(arguments->listFile,
                    "List file not found");

                // Count number of file entries, atleast 2 are needed
                LoggedAssert(getNumberOfLinesFromFilename(arguments->listFile)
                    >1, "File list invalid, atleast two entries are required");

            } else {
                // First path is executable
                arguments->numberOfPaths = state->arg_num;
                arguments->filePaths = state->argv + state->argc -\
                    arguments->numberOfPaths;

                for (unsigned int i = 0; i < arguments->numberOfPaths; ++i) {
                    AssertFileExistence(arguments->filePaths[i],
                        "File %s not found, aborting",
                        arguments->filePaths[i]);
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
        { "verbosity", 'v', "{1..7}", 0, "Increase output verbosity"},
        { "multithread", 'p', 0, OPTION_ARG_OPTIONAL, "Enable multithreaded processing"},
        { "lookup_mode", 'm', "{fast,full}", 0, "Calculate the matching for "
            "the whole video and output whether the whole video matches or "
            "only parts, or Calculate only until a matching is found or the "
            "video ends. Should be faster in some cases."},
        { "signature_type", 't', "{xml,binary}", 0, "Only binary is supported"},
        { "minimum_score", 'k', "{float}", 0, "The minimum score to meet to be shown as similar"
            "The default value is 9000"},
        { "thD", 'd', "{float}", 0, "Threshold to detect one word as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 9000."},
        { "thDc", 'c', "{float}", 0, "Threshold to detect all words as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 60000."},
        { "thXh", 'x', "{float}", 0, "Threshold to detect frames as similar. The "\
            "option value must be an integer greater than zero. The default "\
            "value is 290."},
        { "thDi", 'i', "{float}", 0, "The minimum length of a sequence in frames to "\
            "recognize it as matching sequence. The option value must be a "\
            "non negative integer value. The default value is 150."},
        { "thIt", 'b', "{float}", 0, "Threshold for relation of good to all frames."\
            "The option value must be a double "\
            "value between 0 and 1. The default value is 0.5."},
        { "output_format", 'f', "{csv,beautiful}", 0, "The desired output format. "\
            "Only csv and beautiful are supported. beautiful is default"},
        { "file_list", 'l', "file_list", 0, "Specify a list of signature files"},
        { "incremental_file_list", 'n', "incremental_file_list", 0, "Specify a list of signature files that will be matched between each other and the specified files. Use this mode if you DON'T want to rematch every signature in the file list or argument list."},
        { "session_file", 's', "session_file", 0, "Resume previous session"},
        { 0 }
    };

    struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };
    result = argp_parse(&argp, argc, argv, 0, 0, &arguments);
    LoggedAssert(!result, "Argument parsing failed");

    return arguments;
};
