#include "main.h"

struct arguments args = {0};
struct session session = {0};

int
main(int argc, char **argv) {
    struct fileIndex index = {0};
    void (*printFunctionPointer)(MatchingInfo *info, StreamContext* sc,\
        char *file1, char *file2, int isFirst, int isLast, int isMoreThanOne)\
        = printBeautiful;


    slog_init("logfile", "slog.cfg", 5, 1);
    initSession(&session, &args, &index);


    if (args.sessionFile)
        loadSession(&args, &index, args.sessionFile);

    signal(SIGINT, INThandler);

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

    if (args.useOpenMp)
        #pragma omp parallel
        {
            #pragma omp single
            slog_info(4, "Using %d threads", omp_get_num_threads());
        }

    if (args.outputFormat == CSV) {
        printCSVHeader();
        printFunctionPointer = printCSV;
    } else {
        printBeautifulHeader();
        printFunctionPointer = printBeautiful;
    }

    if (args.listFile) {
        initFileIterator(&index, args.listFile);
        processFiles(&index, printFunctionPointer, args.useOpenMp);
        terminateFileIterator(&index);
    } else {
        initFileIteratorFromCmdLine(&index, args.filePaths,\
            args.numberOfPaths);
        processFiles(&index, printFunctionPointer, args.useOpenMp);
        terminateFileIterator(&index);
    }

    if (args.sessionFile)
        deleteSession(args.sessionFile);
    slog_info(4, "Signature processing finished");

    return 0;
}


void
processFiles(struct fileIndex *index, void (*printFunctionPointer)
    (MatchingInfo *info, StreamContext* sc, char *file1, char *file2, \
     int isFirst, int isLast, int isMoreThanOne), int useOpenMp) {

    for (unsigned int i = index->indexA + 1; i < index->maxIndex; ++i) {
        StreamContext scontextsBase[NUM_OF_INPUTS] = { 0 };
        char *file1 = &index->pathsMatrix[i*MAX_PATH_LENGTH];
        binary_import(&scontextsBase[0], file1);

        // Used for session saving
        index->indexA = i - 1;

        #pragma omp parallel for ordered schedule(dynamic) if(useOpenMp)
        for (unsigned int j = index->indexB + 1; j < index->maxIndex; ++j) {

            // Used for session saving
            #pragma omp ordered
            index->indexB = j - 1;

            struct fileIndex tmpIndex = {
                .indexA = i,
                .indexB = j,
                .maxIndex = index->maxIndex,
                .pathsMatrix = index->pathsMatrix
            };
            StreamContext scontexts[NUM_OF_INPUTS];
            MatchingInfo result = {0};
            char *file2 = &tmpIndex.pathsMatrix[tmpIndex.indexB*MAX_PATH_LENGTH];

            scontexts[0] = scontextsBase[0];
            binary_import(&scontexts[1], file2);

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

            result = processSignaturePair(&scontexts[0], &scontexts[1],
                sigContext);
            printResult(&tmpIndex, &result, &sigContext, args.minScore,
                printFunctionPointer);

            signature_unload(&scontexts[1]);
        }
        signature_unload(&scontextsBase[0]);
    }
}

// This function processes the signatures by using index as an iterator
void
processFilePair(
    struct fileIndex *index,
    void (*printFunctionPointer)
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
    binary_import(&scontexts[1], filePath2);

    slog_debug(6, "Processing %s\t%s", filePath1, filePath2);

    result = processSignaturePair(&scontexts[0], &scontexts[1], sigContext);
    printResult(index, &result, &sigContext, args.minScore,
        printFunctionPointer);

    signature_unload(&scontexts[1]);
    signature_unload(&scontexts[0]);
}

MatchingInfo
processSignaturePair(
    struct StreamContext *signatureA,
    struct StreamContext *signatureB,
    struct SignatureContext sigContext) {

    MatchingInfo result = {0};

    printStreamContext(signatureA);
    printStreamContext(signatureB);

    result = lookup_signatures(&sigContext, signatureA, signatureB);
    return result;
}

void  INThandler(int sig)
{
     char  c;

     signal(sig, SIG_IGN);
     printf(" detected Do you really want to quit or save the session?"
        " [Yes/No/Save] ");
     c = getchar();
     switch (c) {
         case 'y':
         case 'Y':
             exit(0);
             break;

         case 's':
         case 'S':
             saveSessionPrompt(&session);
             exit(0);
             break;

         case 'n':
         case 'N':
         default:
             break;
     }
     // We have to reset the handler after every catch
     signal(SIGINT, INThandler);
     getchar(); // Get new line character
}
