#include "main.h"

void printFineSigList(FineSignature *list, FineSignature *end, int lastCoarse) {

    for (FineSignature *i = list; i != end; i = i->next) {
        if (lastCoarse) {
            if (i->next != end) {
                slog_debug(6,"  \u2523\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"  \u2503 \u2517\u2501 Pts: %lu\tIndex %d \t"\
                    "Confidence: %hhu", i->pts, i->index, i->confidence);
            } else {
                slog_debug(6,"  \u2517\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"    \u2517\u2501 Pts: %lu\tIndex %d \t"\
                    "Confidence: %hhu", i->pts, i->index, i->confidence);
            }
        } else {
            if (i->next != end) {
                slog_debug(6,"\u2503 \u2523\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"\u2503 \u2503 \u2517\u2501 Pts: %lu\tIndex %d \t"\
                    "Confidence: %hhu", i->pts, i->index, i->confidence);
            } else {
                slog_debug(6,"\u2503 \u2517\u2501\u2533 Fine signature at %p", i);
                slog_debug(6,"\u2503   \u2517\u2501 Pts: %lu\tIndex %d \t"\
                    "Confidence: %hhu", i->pts, i->index, i->confidence);
            }
        }
    }
}

void printCoarseSigList(CoarseSignature *list) {
    for (CoarseSignature *j = list; j->next ; j = j->next) {
        if (j->next->next) {
            slog_debug(6,"\u2523\u2533 Coarse signature at %p", j);
            slog_debug(6,"\u2503\u2517\u2533\u2578 Fine signatures "\
                "bounds: %p %p", list->first, list->last);
            printFineSigList(list->first, list->last, 0);
        } else {
            slog_debug(6,"\u2517\u2533 Coarse signature at %p", j);
            slog_debug(6," \u2517\u2533\u2578 Fine signatures "\
                "bounds: %p %p", list->first, list->last);
            printFineSigList(list->first, list->last, 1);
        }
    }
}

void printStreamContext(StreamContext* sc) {
    slog_debug(6,"\u250F Time base: %d/%d", sc->time_base.num,sc->time_base.den);
    slog_debug(6,"\u2523 Width: %d\tHeight: %d",sc->w,sc->h);
    slog_debug(6,"\u2523 Overflow protection: %d", sc->divide);
    slog_debug(6,"\u2523 Fine signatures list: %p", sc->finesiglist);
    slog_debug(6,"\u2523 Coarse signature list: %p", sc->coarsesiglist);
    slog_debug(6,"\u2523 Last index: %d", sc->lastindex);
    slog_debug(6,"\u2523 Signatures:", sc->lastindex);
    printCoarseSigList(sc->coarsesiglist);
}

int
fineSignatureCmp(const void* p1, const void* p2) {
    FineSignature *a = (FineSignature*) p1;
    FineSignature *b = (FineSignature*) p2;
    if (a->pts == b->pts)
        return 0;
    else if (a->pts < b->pts)
        return -1;
    else
        return 1;
};

StreamContext*
binary_import(const char* filename)
{
    FILE *f = NULL;
    unsigned int rResult = 0, fileLength = 0, paddedLength = 0,\
        numOfSegments = 0;
    uint8_t *buffer = NULL;
    StreamContext *sc = NULL;
    GetBitContext bitContext = { 0 };

    slog_debug(0, "Loading signature from: %s", filename);

    sc = (StreamContext*) calloc(1, sizeof(StreamContext));
    Assert(sc);

    f = fopen(filename, "rb");
    LoggedAssert(f, "Can't open %s", filename);

    // We get to total file length
    fseek(f, 0, SEEK_END);
    fileLength = ftell(f);
    LoggedAssert(fileLength > 0, "Input file is empty");
    fseek(f, 0, SEEK_SET);

    // Cast to float is necessary to avoid int division
    paddedLength = ceil(fileLength / (float) AV_INPUT_BUFFER_PADDING_SIZE)*\
                   AV_INPUT_BUFFER_PADDING_SIZE;
    buffer = (uint8_t*) calloc(paddedLength, sizeof(uint8_t));
    LoggedAssert(buffer, "Could not allocate memory buffer");

    // Read entire file into memory
    rResult = fread(buffer, sizeof(uint8_t), fileLength, f);
    Assert(rResult == fileLength);
    // Remove FILE pointer from memory once we're done
    fclose(f);
    f = NULL;

    init_get_bits(&bitContext, buffer, paddedLength);
    // libavcodec

    // Skip the following data:
    // - NumOfSpatial Regions: (32 bits) only 1 supported
    // - SpatialLocationFlag: (1 bit) always the whole image
    // - PixelX_1: (16 bits) always 0
    // - PixelY_1: (16 bits) always 0
    skip_bits(&bitContext, 32 + 1 + 16*2);


	// width - 1, and height - 1
    // PixelX_2: (16 bits) is width - 1
    // PixelY_2: (16 bits) is height - 1
    sc->w = get_bits(&bitContext, 16);
    sc->h = get_bits(&bitContext, 16);
    ++sc->w;
    ++sc->h;

    // StartFrameOfSpatialRegion, always 0
    skip_bits(&bitContext, 32);

    // NumOfFrames
    sc->lastindex = get_bits(&bitContext, 32);

    // sc->time_base.den / sc->time_base.num
    // hoping num is 1, other values are vague
    // den/num might be greater than 16 bit, so cutting it
    //put_bits(&buf, 16, 0xFFFF & (sc->time_base.den / sc->time_base.num));
    // MediaTimeUnit

    sc->time_base.den = get_bits(&bitContext, 16);
	sc->time_base.num = 1;

    // Skip the following data
    // - MediaTimeFlagOfSpatialRegion: (1 bit) always 1
    // - StartMediaTimeOfSpatialRegion: (32 bits) always 0
    skip_bits(&bitContext, 1 + 32);

    // EndMediaTimeOfSpatialRegion
    uint64_t lastCoarsePts = get_bits(&bitContext, 32);

    // Coarse signatures
    // numOfSegments = number of coarse signatures
    // Reading from binary signature return a wrong number of segments
    numOfSegments = (sc->lastindex + 44)/45;
    skip_bits(&bitContext, 32);

	sc->coarsesiglist = (CoarseSignature*) calloc(numOfSegments,\
            sizeof(CoarseSignature));

	BoundedCoarseSignature *bCoarseList = (BoundedCoarseSignature*)\
        calloc(numOfSegments, sizeof(BoundedCoarseSignature));

    LoggedAssert(sc->coarsesiglist, "Failed to create coarse signature list");
    LoggedAssert(bCoarseList, "Failed to create bounded "\
            "coarse signature list");


    // CoarseSignature loading
    slog_debug(6, "Loading %4d coarse signatures from: %s", numOfSegments,\
        filename);
    for (unsigned int i = 0; i < numOfSegments; ++i) {
        BoundedCoarseSignature *bCs = &bCoarseList[i];
        bCs->cSign = &sc->coarsesiglist[i];

        if (i < numOfSegments - 1)
            bCs->cSign->next = &sc->coarsesiglist[i + 1];

        // each coarse signature is a VSVideoSegment

        // StartFrameOfSegment
        bCs->firstIndex = get_bits(&bitContext, 32);
        // EndFrameOfSegment
        bCs->lastIndex = get_bits(&bitContext, 32);

        // MediaTimeFlagOfSegment 1 bit, always 1
        skip_bits(&bitContext, 1);

        // Fine signature pts
        // StartMediaTimeOfSegment 32 bits
        bCs->firstPts = get_bits(&bitContext, 32);
        // EndMediaTimeOfSegment 32 bits
        bCs->lastPts = get_bits(&bitContext, 32);


		// Bag of words
        for (unsigned int i = 0; i < 5; ++i) {
            // read 243 bits ( = 7 * 32 + 19 = 8 * 28 + 19) into buffer

            for (unsigned int j = 0; j < 30; ++j) {
                // 30*8 bits = 30 bytes
                bCs->cSign->data[i][j] = get_bits(&bitContext, 8);
            }
            bCs->cSign->data[i][30] = get_bits(&bitContext, 3) << 5;
        }
        slog_debug(6, "indexes: %010lu|%010lu\tpts: %010llu|%010llu"\
            "\tnext: %p", bCs->firstIndex, bCs->lastIndex, bCs->firstPts,
            bCs->lastPts, bCs->cSign->next);
    }
    sc->coarseend = &sc->coarsesiglist[numOfSegments-1];

    // Finesignatures
    // CompressionFlag, only 0 supported
    skip_bits(&bitContext, 1);


    sc->finesiglist = (FineSignature*) calloc(sc->lastindex,\
            sizeof(FineSignature));
    LoggedAssert(sc->finesiglist,\
        "Could not allocate FineSignatures memory buffer");

    // Load fine signatures from file
    slog_debug(6, "Loading %4d fine signatures from: %s", sc->lastindex,\
        filename);
    for (unsigned int i = 0; i < sc->lastindex; ++i) {
        FineSignature *fs = &sc->finesiglist[i];

        // MediaTimeFlagOfFrame always 1
        skip_bits(&bitContext, 1);

        // MediaTimeOfFrame (PTS)
        fs->pts = 0xFFFFFFFF & get_bits(&bitContext, 32);

        // FrameConfidence
        fs->confidence = get_bits(&bitContext, 8);

        // words
        for (unsigned int l = 0; l < 5; l++) {
            fs->words[l] = get_bits(&bitContext, 8);
        }

        // framesignature
        for (unsigned int l = 0; l < SIGELEM_SIZE/5; l++) {
            fs->framesig[l] = get_bits(&bitContext, 8);
        }
    };

    // Sort by frame time (pts)
    qsort(sc->finesiglist, sc->lastindex, sizeof(FineSignature),\
            fineSignatureCmp);


    // Creating FineSignature linked list
    // CAN CAUSE MEMORY CORRUPTION
    for (unsigned int i = 0; i < sc->lastindex; ++i) {
        FineSignature *fs = &sc->finesiglist[i];
        // Building fine signature list
        // First element prev should be NULL
        // Last element next should be NULL
        if (i == 0) {
            fs->next = &fs[1];
            fs->prev = NULL;
        } else if (i == sc->lastindex - 1) {
            fs->next = NULL;
            fs->prev = &fs[-1];
        } else {
            fs->next = &fs[1];
            fs->prev = &fs[-1];
        }

        slog_debug(6, "pts: %010llu\tconfidence: %03hhu\tnext: %10p"\
            "\tprev: %10p", fs->pts, fs->confidence, fs->next, fs->prev);
    }

    int counter = 0;
    // Fine signature ranges don't overlap
    // so by keeping the last used fine signature index
    // we save a few cycles
    unsigned int lastUsedFineSigIndex = 0;
    // Assign FineSignatures to CoarseSignature s
    for (unsigned int i = 0; i < numOfSegments; ++i) {
        BoundedCoarseSignature *bCs = &bCoarseList[i];
        // O = n^2 probably it can be done faster
        for (;lastUsedFineSigIndex < sc->lastindex; ++lastUsedFineSigIndex) {
            FineSignature *fs = &sc->finesiglist[lastUsedFineSigIndex];

            // THIS CODE IS WRONG only 6 fine signatures get processed
            printf("%010lu %010lu %010lu\n", fs->pts, bCs->firstPts, bCs->lastPts);
            if (fs->pts >= bCs->firstPts && fs->pts <= bCs->lastPts) {
                ++counter;

                if (bCs->cSign->first) {
                    if (bCs->cSign->first->pts > fs->pts) {
                        bCs->cSign->first = fs;
                    }
                } else {
                    bCs->cSign->first = fs;
                }
                if (bCs->cSign->last) {
                        if (bCs->cSign->last->pts < fs->pts)
                            bCs->cSign->last = fs;
                } else {
                    bCs->cSign->last = fs;
                }
            }
        }
        // If atleast one is not NULL then the signature is valid
        // otherwise it isn't
        /*LoggedAssert(bCs->cSign->first || bCs->cSign->last,\
            "Empty coarse signature");*/
    };

    printf("%d\n", counter);
    //sc->coarseend->last->pts = lastCoarsePts;
    //sc->coarseend->first->pts = 0;

    free(bCoarseList);
    free(buffer);
    return sc;
}


int
main(int argc, char **argv) {
    StreamContext *a, *b, *c;
    MatchingInfo result = {0};
	struct arguments options;
    // 0    panic
    // 1    fatal
    // 2    error
    // 3    warn
    // 4    info
    // 5    live
    // 6    debug
    #ifdef DEBUG
        slog_init("logfile", "slog.cfg", 6, 1);
    #else
        slog_init("logfile", "slog.cfg", 5, 1);
    #endif
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
