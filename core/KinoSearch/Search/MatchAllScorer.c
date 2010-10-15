#define C_KINO_MATCHALLSCORER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/MatchAllScorer.h"

MatchAllScorer*
MatchAllScorer_new(float score, int32_t doc_max)
{
    MatchAllScorer *self 
        = (MatchAllScorer*)VTable_Make_Obj(MATCHALLSCORER);
    return MatchAllScorer_init(self, score, doc_max);
}

MatchAllScorer*
MatchAllScorer_init(MatchAllScorer *self, float score, int32_t doc_max)
{
    Matcher_init((Matcher*)self);
    self->doc_id        = 0;
    self->score         = score;
    self->doc_max       = doc_max;
    return self;
}

int32_t
MatchAllScorer_next(MatchAllScorer* self) 
{
    if (++self->doc_id <= self->doc_max) {
        return self->doc_id;
    }
    else {
        self->doc_id--;
        return 0;
    }
}

int32_t
MatchAllScorer_advance(MatchAllScorer* self, int32_t target) 
{
    self->doc_id = target - 1;
    return MatchAllScorer_next(self);
}

float
MatchAllScorer_score(MatchAllScorer* self) 
{
    return self->score;
}

int32_t 
MatchAllScorer_get_doc_id(MatchAllScorer* self) 
{
    return self->doc_id;
}


