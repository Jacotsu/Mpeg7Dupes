#ifndef MAIN
#define MAIN

#include <float.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "slog.h"



#define COARSE_SIZE 90
#define SIGELEM_SIZE 380
#define MAX_FRAMERATE 60
#define HOUGH_MAX_OFFSET 90
#define DIR_NEXT 1
#define DIR_NEXT_END 3
#define DIR_PREV 0
#define DIR_PREV_END 2
#define STATUS_NULL 0
#define STATUS_END_REACHED 1
#define STATUS_BEGIN_REACHED 2

#define av_popcount __builtin_popcount
#define FFMAX(a,b) ((a) > (b) ? (a) : (b))

/*

enum lookup_mode {
    MODE_OFF,
    MODE_FULL,
    MODE_FAST,
    NB_LOOKUP_MODE
};

typedef struct CoarseSignature {
    uint8_t data[5][31]; // 5 words with min. 243 bit
    struct FineSignature* first; // associated Finesignatures
    struct FineSignature* last;
    struct CoarseSignature* next;
} CoarseSignature;

typedef struct FineSignature {
    struct FineSignature* next;
    struct FineSignature* prev;
    uint64_t pts;
    uint32_t index; // needed for xmlexport
    uint8_t confidence;
    uint8_t words[5];
    uint8_t framesig[SIGELEM_SIZE/5];
} FineSignature;

typedef struct MatchingInfo {
    double meandist;
    double framerateratio; // second/first
    int score;
    int offset;
    int matchframes; // number of matching frames
    int whole;
    struct FineSignature* first;
    struct FineSignature* second;
    struct MatchingInfo* next;
} MatchingInfo;

typedef struct StreamContext {
    //AVRational time_base;
    int time_base;
    // needed for xml_export
    int w; // height
    int h; // width

    // overflow protection
    int divide;

    FineSignature* finesiglist;
    FineSignature* curfinesig;

    CoarseSignature* coarsesiglist;
    CoarseSignature* coarseend; // needed for xml export
    // helpers to store the alternating signatures
    CoarseSignature* curcoarsesig1;
    CoarseSignature* curcoarsesig2;

    int coarsecount; // counter from 0 to 89
    int midcoarse;   // whether it is a coarsesignature beginning from 45 + i * 90
    uint32_t lastindex; // helper to store amount of frames

    int exported; // boolean whether stream already exported
} StreamContext;

typedef struct SignatureContext {
    //const AVClass *class;
    const int *class;
    // input parameters
    int mode;
    int nb_inputs;
    char *filename;
    int format;
    int thworddist;
    int thcomposdist;
    int thl1;
    int thdi;
    int thit;
    // end input parameters

    uint8_t l1distlut[243*242/2]; // 243 + 242 + 241 ...
    StreamContext* streamcontexts;
} SignatureContext;
*/

#endif
