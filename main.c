#include "main.h"


int
fineSignatureCmp(const void* p1, const void* p2) {
    if (((FineSignature*) p1)->pts <= ((FineSignature*) p2)->pts) return -1;
    if (((FineSignature*) p1)->pts == ((FineSignature*) p2)->pts) return 0;
    if (((FineSignature*) p1)->pts >= ((FineSignature*) p2)->pts) return -1;

    LoggedAssert(0, "Fine signature compare didn't return a valid value");
};

StreamContext*
binary_import(const char* filename)
{
    FILE *f = NULL;
    unsigned int rResult = 0, fileLength = 0, paddedLength = 0;
    uint8_t *buffer = NULL;

    StreamContext *sc = (StreamContext*) calloc(1, sizeof(StreamContext));
    Assert(sc);
    memset(sc, 0, sizeof(StreamContext));

    f = fopen(filename, "rb");
    if (!f) {
        slog_error(0, "Can't open %s", filename);
		exit(1);
    }


    // We get to total file length
    fseek(f, 0, SEEK_END);
    fileLength = ftell(f);
    LoggedAssert(fileLength > 0, "Input file is empty");
    fseek(f, 0, SEEK_SET);

    // Cast to float is necessary to avoid int division
    paddedLength = ceil(fileLength / (float) AV_INPUT_BUFFER_PADDING_SIZE)*\
                   AV_INPUT_BUFFER_PADDING_SIZE;
    buffer = (uint8_t*) calloc(1, paddedLength);
    LoggedAssert(buffer, "Could not allocate memory buffer");

    // Read entire file into memory
    rResult = fread(buffer, 1, fileLength, f);
    Assert(rResult == fileLength);
    // Remove FILE pointer from memory once we're done
    fclose(f);


    GetBitContext bitContext = { 0 };
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
    unsigned int numOfSegments = get_bits(&bitContext, 32);

	sc->coarsesiglist = (CoarseSignature*) calloc(numOfSegments,\
            sizeof(CoarseSignature));
	BoundedCoarseSignature *bCoarseList = (BoundedCoarseSignature*)\
        calloc(numOfSegments, sizeof(CoarseSignature));

    LoggedAssert(sc->coarsesiglist, "Failed to create coarse signature list");
    LoggedAssert(bCoarseList, "Failed to create bounded "\
            "coarse signature list");

    memset(sc->coarsesiglist, 0, numOfSegments*sizeof(CoarseSignature));
    memset(bCoarseList, 0, numOfSegments*sizeof(BoundedCoarseSignature));

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
            rResult = fread(&bCs->cSign->data[i][0], sizeof(uint8_t), 31, f);

            for (unsigned int j = 0; j < 30; ++j) {
                // 30*8 bits = 30 bytes
                bCs->cSign->data[i][j] = get_bits(&bitContext, 8);
            }
            Assert(rResult == 31);
            bCs->cSign->data[i][30] = get_bits(&bitContext, 3) << 5;
        }
    }
    sc->coarseend = &sc->coarsesiglist[numOfSegments-1];

    // Finesignatures
    // CompressionFlag, only 0 supported
    skip_bits(&bitContext, 1);


    sc->finesiglist = (FineSignature*) calloc(sc->lastindex,\
            sizeof(FineSignature));
    memset(sc->finesiglist, 0, sc->lastindex*sizeof(FineSignature));

    // Load fine signatures from file
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

        /* framesignature */
        for (unsigned int l = 0; l < SIGELEM_SIZE/5; l++) {
            fs->framesig[l] = get_bits(&bitContext, 8);
        }
    };

    // Sort by frame time (pts)
    qsort(sc->finesiglist, sc->lastindex, sizeof(FineSignature),\
            fineSignatureCmp);


    for (unsigned int i = 0; i < sc->lastindex; ++i) {
        FineSignature *fs = &sc->finesiglist[i];
        // Building fine signature list
        // First element prev should be NULL
        // Last element next should be NULL
        if (i < sc->lastindex - 1) {
            if (i == 0) {
                fs[0].next = &fs[1];
            } else {
                fs[i].next = &fs[i+1];
                fs[i].prev = &fs[i-1];
            }
        } else {
                fs[i].next = &fs[i+1];
                fs[i+1].prev = &fs[i];
        }
    }

    // Assign FineSignatures to CoarseSignature s
    for (unsigned int i = 0; i < numOfSegments; ++i) {
        BoundedCoarseSignature *bCs = &bCoarseList[i];
        // O = n^2 probably it can be done faster
        for (unsigned int j = 0; j < sc->lastindex; ++j) {
            FineSignature *fs = &sc->finesiglist[j];
            if (fs->pts >= bCs->firstIndex && fs->pts <= bCs->lastIndex) {
                if (bCs->cSign->first) {
                    if (bCs->cSign->first->pts > fs->pts)
                        bCs->cSign->first = fs;
                } else {
                    bCs->cSign->first = fs;
                }
            }

            if (fs->pts >= bCs->firstIndex && fs->pts <= bCs->lastIndex) {
                if (bCs->cSign->last) {
                    if (bCs->cSign->last->pts > fs->pts)
                        bCs->cSign->last = fs;
                } else {
                    bCs->cSign->last = fs;
                }
            }
        }
    };


    free(bCoarseList);
    free(buffer);

    return sc;
}


int
main(int argc, char **argv) {
    StreamContext *a, *b;
    MatchingInfo result = {0};
	struct arguments options;

    slog_init("logfile", "slog.cfg", 10, 1);
	options = parseArguments(argc, argv);


	SignatureContext sigContext = {
        .class = NULL,
        .mode = MODE_FULL,
        .nb_inputs = 1,
        .filename = "",
        .thworddist = 0,
        .thcomposdist = 0,
        .thl1 = 0,
        .thdi = 0,
        .thit = 0
    };


    a = binary_import("1234_In_the_name_of_GodCCS_tarrant.webm.sig");
    b = binary_import("1234_In_the_name_of_GodCCS_tarrant.webm.sig");


    //static MatchingInfo
	//lookup_signatures(
    //SignatureContext *sc,
    //StreamContext *first,
    //StreamContext *second,
    //int mode)
    //result = lookup_signatures(signatureContext, streaContext1, streamContext2, mode);
    result = lookup_signatures(&sigContext, a, b, sigContext.mode);

    return 0;
}
