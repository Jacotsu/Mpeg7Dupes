#include "signature_load.h"


void
binary_import(StreamContext *sc, const char* filename)
{
    FILE *f = NULL;
    unsigned int rResult = 0, fileLength = 0, paddedLength = 0,\
        numOfSegments = 0;
    uint8_t *buffer = NULL;
    GetBitContext bitContext = { 0 };

    slog_debug(6, "Loading signature from: %s", filename);

    Assert(sc);

    f = fopen(filename, "rb");
    LoggedAssert(f, "Can't open %s", filename);

    // We get to total file length
    fileLength = getFileSize(filename);

    // Cast to float is necessary to avoid int division
    paddedLength = ceil(fileLength / (float) AV_INPUT_BUFFER_PADDING_SIZE)*\
                   AV_INPUT_BUFFER_PADDING_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
    buffer = (uint8_t*) calloc(paddedLength, sizeof(uint8_t));
    LoggedAssert(buffer, "Could not allocate memory buffer");

    // Read entire file into memory
    rResult = fread(buffer, sizeof(uint8_t), fileLength, f);
    Assert(rResult == fileLength);
    // Remove FILE pointer from memory once we're done
    fclose(f);
    f = NULL;

    // BE CAREFUL, THE LENGTH IS SPECIFIED IN BITS NOT BYTES
    init_get_bits(&bitContext, buffer, 8*fileLength);
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

        // Crashes for some signature, it's a memory adding problems
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

    // Fine signature ranges DO overlap
    // Assign FineSignatures to CoarseSignature s
    for (unsigned int i = 0; i < numOfSegments; ++i) {
        BoundedCoarseSignature *bCs = &bCoarseList[i];
        // O = n^2 probably it can be done faster
        for (unsigned int j = 0;  j < sc->lastindex  &&\
            sc->finesiglist[j].pts <= bCs->lastPts; ++j) {
            FineSignature *fs = &sc->finesiglist[j];

            if (fs->pts >= bCs->firstPts) {
                //slog_debug(6, "%d %d %d",bCs->firstPts, fs->pts, bCs->lastPts);
                // Check if the fragment's pts is inside coarse signature
                // bounds. Upper bound is checked in for loop
                if (!bCs->cSign->first) {
                    bCs->cSign->first = fs;
                }
                if (bCs->cSign->last) {
                    if (bCs->cSign->last->pts <= fs->pts)
                        bCs->cSign->last = fs;
                } else {
                    bCs->cSign->last = fs;
                }
                slog_debug(6, "[%5d -> %5d - |%5d| - %5d <- %5d]",\
                    bCs->firstPts, bCs->cSign->first->pts,\
                    fs->pts, bCs->cSign->last->pts, bCs->lastPts);
            }
        }
        // If atleast one is not NULL then the signature is valid
        // otherwise it isn't
        //slog_warn(3, "Empty coarse signature in signature: %s", filename);
        //LoggedAssert(bCs->cSign->first || bCs->cSign->last,\
        //   "Empty coarse signature");
    };
    // Apparently there can be empty coarse signatures
    if (sc->coarseend->last)
        sc->coarseend->last->pts = lastCoarsePts;
    if (sc->coarseend->first)
        sc->coarseend->first->pts = 0;

    free(bCoarseList);
    free(buffer);
}

void
signature_unload(StreamContext *sc)
{
    free(sc->coarsesiglist);
    free(sc->finesiglist);
}
