#include "main.h"

/*

static unsigned int intersection_word(
    const uint8_t *first,
    const uint8_t *second)
{
    unsigned int val=0,i;
    for (i = 0; i < 28; i += 4) {
        val += av_popcount( (first[i]   & second[i]  ) << 24 |
                            (first[i+1] & second[i+1]) << 16 |
                            (first[i+2] & second[i+2]) << 8  |
                            (first[i+3] & second[i+3]) );
    }
    val += av_popcount( (first[28] & second[28]) << 16 |
                        (first[29] & second[29]) << 8  |
                        (first[30] & second[30]) );
    return val;
}

static unsigned int union_word(
    const uint8_t *first,
    const uint8_t *second)
{
    unsigned int val=0,i;
    for (i = 0; i < 28; i += 4) {
        val += av_popcount( (first[i]   | second[i]  ) << 24 |
                            (first[i+1] | second[i+1]) << 16 |
                            (first[i+2] | second[i+2]) << 8  |
                            (first[i+3] | second[i+3]) );
    }
    val += av_popcount( (first[28] | second[28]) << 16 |
                        (first[29] | second[29]) << 8  |
                        (first[30] | second[30]) );
    return val;
}

static int get_jaccarddist(
    SignatureContext *sc,
    CoarseSignature *first,
    CoarseSignature *second)
{
    int jaccarddist, i, composdist = 0, cwthcount = 0;
    for (i = 0; i < 5; i++) {
        if ((jaccarddist = intersection_word(first->data[i],
                second->data[i])) > 0) {
            jaccarddist /= union_word(first->data[i], second->data[i]);
        }
        if (jaccarddist >= sc->thworddist) {
            if (++cwthcount > 2) {
                // more than half (5/2) of distances are too wide 
                return 0;
            }
        }
        composdist += jaccarddist;
        if (composdist > sc->thcomposdist) {
            return 0;
        }
    }
    return 1;
}

static unsigned int get_l1dist(
    SignatureContext *sc,
    const uint8_t *first,
    const uint8_t *second)
{
    unsigned int i;
    unsigned int dist = 0;
    uint8_t f, s;

    for (i = 0; i < SIGELEM_SIZE/5; i++) {
        if (first[i] != second[i]) {
            f = first[i];
            s = second[i];
            if (f > s) {
                // little variation of gauss sum formula
                dist += sc->l1distlut[243*242/2 - (243-s)*\
                        (242-s)/2 + f - s - 1];
            } else {
                dist += sc->l1distlut[243*242/2 - (243-f)*\
                        (242-f)/2 + s - f - 1];
            }
        }
    }
    return dist;
}


static MatchingInfo* get_matching_parameters(
        SignatureContext *sc,
        FineSignature *first,
        FineSignature *second)
{
    FineSignature *f, *s;
    size_t i, j, k, l, hmax = 0, score;
    int framerate, offset, l1dist;
    double m;
    MatchingInfo *cands = NULL, *c = NULL;

    struct {
        uint8_t size;
        unsigned int dist;
        FineSignature *a;
        uint8_t b_pos[COARSE_SIZE];
        FineSignature *b[COARSE_SIZE];
    } pairs[COARSE_SIZE];

    typedef struct hspace_elem {
        int dist;
        size_t score;
        FineSignature *a;
        FineSignature *b;
    } hspace_elem;

    // houghspace 
    hspace_elem** hspace = (hspace_elem**) \
        calloc(MAX_FRAMERATE, sizeof(hspace_elem *));

    // initialize houghspace
    for (i = 0; i < MAX_FRAMERATE; i++) {
        hspace[i] = (hspace_elem*) \
            calloc(2 * HOUGH_MAX_OFFSET + 1, sizeof(hspace_elem));
        for (j = 0; j < HOUGH_MAX_OFFSET; j++) {
            hspace[i][j].score = 0;
            hspace[i][j].dist = 99999;
        }
    }

    // l1 distances
    for (i = 0, f = first; i < COARSE_SIZE && f->next; i++, f = f->next) {
        pairs[i].size = 0;
        pairs[i].dist = 99999;
        pairs[i].a = f;
        for (j = 0, s = second; j < COARSE_SIZE && s->next; j++, s = s->next) {
            // l1 distance of finesignature
            l1dist = get_l1dist(sc, f->framesig, s->framesig);
            if (l1dist < sc->thl1) {
                if (l1dist < pairs[i].dist) {
                    pairs[i].size = 1;
                    pairs[i].dist = l1dist;
                    pairs[i].b_pos[0] = j;
                    pairs[i].b[0] = s;
                } else if (l1dist == pairs[i].dist) {
                    pairs[i].b[pairs[i].size] = s;
                    pairs[i].b_pos[pairs[i].size] = j;
                    pairs[i].size++;
                }
            }
        }
    }
    // last incomplete coarsesignature
    if (f->next == NULL) {
        for (; i < COARSE_SIZE; i++) {
            pairs[i].size = 0;
            pairs[i].dist = 99999;
        }
    }

    // hough transformation
    for (i = 0; i < COARSE_SIZE; i++) {
        for (j = 0; j < pairs[i].size; j++) {
            for (k = i + 1; k < COARSE_SIZE; k++) {
                for (l = 0; l < pairs[k].size; l++) {
                    if (pairs[i].b[j] != pairs[k].b[l]) {
                        // linear regression
                        m = (pairs[k].b_pos[l]-pairs[i].b_pos[j]) / (k-i); // good value between 0.0 - 2.0 
                        framerate = (int) m*30 + 0.5; // round up to 0 - 60 
                        if (framerate>0 && framerate <= MAX_FRAMERATE) {
                            offset = pairs[i].b_pos[j] - ((int) m*i + 0.5); // only second part has to be rounded up
                            if (offset > -HOUGH_MAX_OFFSET && offset < HOUGH_MAX_OFFSET) {
                                if (pairs[i].dist < pairs[k].dist) {
                                    if (pairs[i].dist < hspace[framerate-1][offset+HOUGH_MAX_OFFSET].dist) {
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].dist = pairs[i].dist;
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].a = pairs[i].a;
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].b = pairs[i].b[j];
                                    }
                                } else {
                                    if (pairs[k].dist < hspace[framerate-1][offset+HOUGH_MAX_OFFSET].dist) {
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].dist = pairs[k].dist;
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].a = pairs[k].a;
                                        hspace[framerate-1][offset+HOUGH_MAX_OFFSET].b = pairs[k].b[l];
                                    }
                                }

                                score = hspace[framerate-1][offset+HOUGH_MAX_OFFSET].score + 1;
                                if (score > hmax )
                                    hmax = score;
                                hspace[framerate-1][offset+HOUGH_MAX_OFFSET].score = score;
                            }
                        }
                    }
                }
            }
        }
    }

    if (hmax > 0) {
        hmax = (int) (0.7*hmax);
        for (i = 0; i < MAX_FRAMERATE; i++) {
            for (j = 0; j < HOUGH_MAX_OFFSET; j++) {
                if (hmax < hspace[i][j].score) {
                    if (c == NULL) {
                        c = (MatchingInfo*) malloc(sizeof(MatchingInfo));
                        if (!c)
                            printf("could not allocate memory");
                        cands = c;
                    } else {
                        c->next = (struct MatchingInfo *) \
                            malloc(sizeof(MatchingInfo));
                        if (!c->next)
                            printf("could not allocate memory");
                        c = c->next;
                    }
                    c->framerateratio = (i+1.0) / 30;
                    c->score = hspace[i][j].score;
                    c->offset = j-90;
                    c->first = hspace[i][j].a;
                    c->second = hspace[i][j].b;
                    c->next = NULL;

                    // not used
                    c->meandist = 0;
                    c->matchframes = 0;
                    c->whole = 0;
                }
            }
        }
    }
    for (i = 0; i < MAX_FRAMERATE; i++) {
        av_freep(&hspace[i]);
    }
    av_freep(&hspace);
    return cands;
}

static MatchingInfo evaluate_parameters(
        SignatureContext *sc,
        MatchingInfo *infos,
        MatchingInfo bestmatch,
        int mode)
{
    int dist, distsum = 0, bcount = 1, dir = DIR_NEXT;
    int fcount = 0, goodfcount = 0, gooda = 0, goodb = 0;
    double meandist, minmeandist = bestmatch.meandist;
    int tolerancecount = 0;
    FineSignature *a, *b, *aprev, *bprev;
    int status = STATUS_NULL;

    for (; infos != NULL; infos = infos->next) {
        a = infos->first;
        b = infos->second;
        while (1) {
            dist = get_l1dist(sc, a->framesig, b->framesig);

            if (dist > sc->thl1) {
                if (a->confidence >= 1 || b->confidence >= 1) {
                    // bad frame (because high different information)
                    tolerancecount++;
                }

                if (tolerancecount > 2) {
                    a = aprev;
                    b = bprev;
                    if (dir == DIR_NEXT) {
                        // turn around
                        a = infos->first;
                        b = infos->second;
                        dir = DIR_PREV;
                    } else {
                        break;
                    }
                }
            } else {
                // good frame
                distsum += dist;
                goodfcount++;
                tolerancecount=0;

                aprev = a;
                bprev = b;

                if (a->confidence < 1) gooda++;
                if (b->confidence < 1) goodb++;
            }

            fcount++;

            dir = iterate_frame(infos->framerateratio, &a, &b, fcount, &bcount, dir);
            if (dir == DIR_NEXT_END) {
                status = STATUS_END_REACHED;
                a = infos->first;
                b = infos->second;
                dir = iterate_frame(infos->framerateratio, &a, &b, fcount, &bcount, DIR_PREV);
            }

            if (dir == DIR_PREV_END) {
                status |= STATUS_BEGIN_REACHED;
                break;
            }

            if (sc->thdi != 0 && bcount >= sc->thdi) {
                break; // enough frames found
            }
        }

        if (bcount < sc->thdi)
            continue; // matching sequence is too short
        if ((double) goodfcount / (double) fcount < sc->thit)
            continue;
        if ((double) goodfcount*0.5 < FFMAX(gooda, goodb))
            continue;

        meandist = (double) goodfcount / (double) distsum;

        if (meandist < minmeandist ||
                status == STATUS_END_REACHED | STATUS_BEGIN_REACHED ||
                mode == MODE_FAST){
            minmeandist = meandist;
            // bestcandidate in this iteration
            bestmatch.meandist = meandist;
            bestmatch.matchframes = bcount;
            bestmatch.framerateratio = infos->framerateratio;
            bestmatch.score = infos->score;
            bestmatch.offset = infos->offset;
            bestmatch.first = infos->first;
            bestmatch.second = infos->second;
            bestmatch.whole = 0; // will be set to true later
            bestmatch.next = NULL;
        }

        // whole sequence is automatically best match
        if (status == (STATUS_END_REACHED | STATUS_BEGIN_REACHED)) {
            bestmatch.whole = 1;
            break;
        }

        // first matching sequence is enough, finding the best one is not necessary
        if (mode == MODE_FAST) {
            break;
        }
    }
    return bestmatch;
}


static int find_next_coarsecandidate(
        SignatureContext *sc,
        CoarseSignature *secondstart,
        CoarseSignature **first,
        CoarseSignature **second,
        int start)
{
    // go one coarsesignature foreword
    if (!start) {
        if ((*second)->next) {
            *second = (*second)->next;
        } else if ((*first)->next) {
            *second = secondstart;
            *first = (*first)->next;
        } else {
            return 0;
        }
    }

    while (1) {
        if (get_jaccarddist(sc, *first, *second))
            return 1;

        // next signature
        if ((*second)->next) {
            *second = (*second)->next;
        } else if ((*first)->next) {
            *second = secondstart;
            *first = (*first)->next;
        } else {
            return 0;
        }
    }
}

static int binary_export(
    AVFilterContext *ctx,
    StreamContext *sc,
    const char* filename)
{
    FILE* f;
    FineSignature* fs;
    CoarseSignature* cs;
    uint32_t numofsegments = (sc->lastindex + 44)/45;
    int i, j;
    PutBitContext buf;
    // buffer + header + coarsesignatures + finesignature
    int len = (512 + 6 * 32 + 3*16 + 2 +
        numofsegments * (4*32 + 1 + 5*243) +
        sc->lastindex * (2 + 32 + 6*8 + 608)) / 8;
    uint8_t* buffer = (uint8_t*) calloc(len, sizeof(uint8_t));
    if (!buffer)
        return AVERROR(ENOMEM);

    f = fopen(filename, "wb");
    if (!f) {
        int err = AVERROR(EINVAL);
        char buf[128];
        av_strerror(err, buf, sizeof(buf));
        av_log(ctx, AV_LOG_ERROR, "cannot open file %s: %s\n", filename, buf);
        av_freep(&buffer);
        return err;
    }
    init_put_bits(&buf, buffer, len);

    put_bits32(&buf, 1); // NumOfSpatial Regions, only 1 supported
    put_bits(&buf, 1, 1); // SpatialLocationFlag, always the whole image
    put_bits32(&buf, 0); // PixelX,1 PixelY,1, 0,0
    put_bits(&buf, 16, sc->w-1 & 0xFFFF); // PixelX,2
    put_bits(&buf, 16, sc->h-1 & 0xFFFF); // PixelY,2
    put_bits32(&buf, 0); // StartFrameOfSpatialRegion
    put_bits32(&buf, sc->lastindex); // NumOfFrames
    // hoping num is 1, other values are vague
    // den/num might be greater than 16 bit, so cutting it
    put_bits(&buf, 16, 0xFFFF & (sc->time_base.den / sc->time_base.num)); // MediaTimeUnit
    put_bits(&buf, 1, 1); // MediaTimeFlagOfSpatialRegion
    put_bits32(&buf, 0); // StartMediaTimeOfSpatialRegion
    put_bits32(&buf, 0xFFFFFFFF & sc->coarseend->last->pts); // EndMediaTimeOfSpatialRegion
    put_bits32(&buf, numofsegments); // NumOfSegments
    // coarsesignatures 
    for (cs = sc->coarsesiglist; cs; cs = cs->next) {
        put_bits32(&buf, cs->first->index); // StartFrameOfSegment
        put_bits32(&buf, cs->last->index); // EndFrameOfSegment
        put_bits(&buf, 1, 1); // MediaTimeFlagOfSegment
        put_bits32(&buf, 0xFFFFFFFF & cs->first->pts); // StartMediaTimeOfSegment
        put_bits32(&buf, 0xFFFFFFFF & cs->last->pts); // EndMediaTimeOfSegment
        for (i = 0; i < 5; i++) {
            // put 243 bits ( = 7 * 32 + 19 = 8 * 28 + 19) into buffer
            for (j = 0; j < 30; j++) {
                put_bits(&buf, 8, cs->data[i][j]);
            }
            put_bits(&buf, 3, cs->data[i][30] >> 5);
        }
    }
    // finesignatures
    put_bits(&buf, 1, 0); // CompressionFlag, only 0 supported
    for (fs = sc->finesiglist; fs; fs = fs->next) {
        put_bits(&buf, 1, 1); // MediaTimeFlagOfFrame
        put_bits32(&buf, 0xFFFFFFFF & fs->pts); // MediaTimeOfFrame
        put_bits(&buf, 8, fs->confidence); // FrameConfidence
        for (i = 0; i < 5; i++) {
            put_bits(&buf, 8, fs->words[i]); // Words
        }
        // framesignature
        for (i = 0; i < SIGELEM_SIZE/5; i++) {
            put_bits(&buf, 8, fs->framesig[i]);
        }
    }

    avpriv_align_put_bits(&buf);
    flush_put_bits(&buf);
    fwrite(buffer, 1, put_bits_count(&buf)/8, f);
    fclose(f);
    av_freep(&buffer);
    return 0;
}

static int export(
    AVFilterContext *ctx,
    StreamContext *sc,
    int input)
{
    SignatureContext* sic = ctx->priv;
    char filename[1024];

    if (sic->nb_inputs > 1) {
        // error already handled
        av_assert0(av_get_frame_filename(filename, sizeof(filename), sic->filename, input) == 0);
    } else {
        if (av_strlcpy(filename, sic->filename, sizeof(filename)) >= sizeof(filename))
            return AVERROR(EINVAL);
    }
    return binary_export(ctx, sc, filename);
}

void match(SignatureContext *sic) {
    if (sic->mode != MODE_OFF) {
        // iterate over every pair
        for (i = 0; i < sic->nb_inputs; i++) {
            sc = &(sic->streamcontexts[i]);
            for (j = i+1; j < sic->nb_inputs; j++) {
                sc2 = &(sic->streamcontexts[j]);
                match = lookup_signatures(ctx, sic, sc, sc2, sic->mode);
                if (match.score != 0) {
                    av_log(ctx, AV_LOG_INFO, "matching of video %d at %f and %d at %f, %d frames matching\n",
                            i, ((double) match.first->pts * sc->time_base.num) / sc->time_base.den,
                            j, ((double) match.second->pts * sc2->time_base.num) / sc2->time_base.den,
                            match.matchframes);
                    if (match.whole)
                        av_log(ctx, AV_LOG_INFO, "whole video matching\n");
                } else {
                    av_log(ctx, AV_LOG_INFO, "no matching of video %d and %d\n", i, j);
                }
            }
        }
    }
}

*/
/*
static int
binary_import(const char* filename)
{
    FILE* f;
    FineSignature* fs;
    CoarseSignature* cs;
    uint32_t numofsegments = (sc->lastindex + 44)/45;
    int i, j;
    PutBitContext buf;
    // buffer + header + coarsesignatures + finesignature
    int len = (512 + 6 * 32 + 3*16 + 2 +
        numofsegments * (4*32 + 1 + 5*243) +
        sc->lastindex * (2 + 32 + 6*8 + 608)) / 8;

    uint8_t* buffer = (uint8_t*) calloc(len, sizeof(uint8_t));

    assert(buffer);
    if (!buffer)
        return AVERROR(ENOMEM);

    f = fopen(filename, "wb");
    if (!f) {
        int err = AVERROR(EINVAL);
        char buf[128];
        av_strerror(err, buf, sizeof(buf));
        av_log(ctx, AV_LOG_ERROR, "cannot open file %s: %s\n", filename, buf);
        av_freep(&buffer);
        return err;
    }
    init_put_bits(&buf, buffer, len);

    put_bits32(&buf, 1); // NumOfSpatial Regions, only 1 supported
    put_bits(&buf, 1, 1); // SpatialLocationFlag, always the whole image
    put_bits32(&buf, 0); // PixelX,1 PixelY,1, 0,0
    put_bits(&buf, 16, sc->w-1 & 0xFFFF); // PixelX,2
    put_bits(&buf, 16, sc->h-1 & 0xFFFF); // PixelY,2
    put_bits32(&buf, 0); // StartFrameOfSpatialRegion
    put_bits32(&buf, sc->lastindex); // NumOfFrames
    // hoping num is 1, other values are vague
    // den/num might be greater than 16 bit, so cutting it
    put_bits(&buf, 16, 0xFFFF & (sc->time_base.den / sc->time_base.num)); // MediaTimeUnit
    put_bits(&buf, 1, 1); // MediaTimeFlagOfSpatialRegion
    put_bits32(&buf, 0); // StartMediaTimeOfSpatialRegion
    put_bits32(&buf, 0xFFFFFFFF & sc->coarseend->last->pts); // EndMediaTimeOfSpatialRegion
    put_bits32(&buf, numofsegments); // NumOfSegments
    // coarsesignatures 
    for (cs = sc->coarsesiglist; cs; cs = cs->next) {
        put_bits32(&buf, cs->first->index); // StartFrameOfSegment
        put_bits32(&buf, cs->last->index); // EndFrameOfSegment
        put_bits(&buf, 1, 1); // MediaTimeFlagOfSegment
        put_bits32(&buf, 0xFFFFFFFF & cs->first->pts); // StartMediaTimeOfSegment
        put_bits32(&buf, 0xFFFFFFFF & cs->last->pts); // EndMediaTimeOfSegment
        for (i = 0; i < 5; i++) {
            // put 243 bits ( = 7 * 32 + 19 = 8 * 28 + 19) into buffer
            for (j = 0; j < 30; j++) {
                put_bits(&buf, 8, cs->data[i][j]);
            }
            put_bits(&buf, 3, cs->data[i][30] >> 5);
        }
    }
    // finesignatures
    put_bits(&buf, 1, 0); // CompressionFlag, only 0 supported
    for (fs = sc->finesiglist; fs; fs = fs->next) {
        put_bits(&buf, 1, 1); // MediaTimeFlagOfFrame
        put_bits32(&buf, 0xFFFFFFFF & fs->pts); // MediaTimeOfFrame
        put_bits(&buf, 8, fs->confidence); // FrameConfidence
        for (i = 0; i < 5; i++) {
            put_bits(&buf, 8, fs->words[i]); // Words
        }
        // framesignature
        for (i = 0; i < SIGELEM_SIZE/5; i++) {
            put_bits(&buf, 8, fs->framesig[i]);
        }
    }

    avpriv_align_put_bits(&buf);
    flush_put_bits(&buf);

    fwrite(buffer, 1, put_bits_count(&buf)/8, f);
    fclose(f);

    free(&buffer);
    return 0;
}*/

int main() {
    slog_init("logfile", "slog.cfg", 10, 1);
    slog(0, SLOG_LIVE, "Test message with level 0");
    slog(1, SLOG_INFO, "Test message with level 0");
    slog(2, SLOG_WARN, "Test message with level 0");
    Assert(0);


    return 0;
}
