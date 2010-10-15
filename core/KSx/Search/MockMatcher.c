#define C_KINO_MOCKMATCHER
#include "KinoSearch/Util/ToolSet.h"

#include "KSx/Search/MockMatcher.h"

MockMatcher*
MockMatcher_new(I32Array *doc_ids, ByteBuf *scores)
{
    MockMatcher *self = (MockMatcher*)VTable_Make_Obj(MOCKMATCHER);
    return MockMatcher_init(self, doc_ids, scores);
}

MockMatcher*
MockMatcher_init(MockMatcher *self, I32Array *doc_ids, ByteBuf *scores)
{
    Matcher_init((Matcher*)self);
    self->tick    = -1;
    self->size    = I32Arr_Get_Size(doc_ids);
    self->doc_ids = (I32Array*)INCREF(doc_ids);
    self->scores  = (ByteBuf*)INCREF(scores);
    return self;
}   

void
MockMatcher_destroy(MockMatcher *self) 
{
    DECREF(self->doc_ids);
    DECREF(self->scores);
    SUPER_DESTROY(self, MOCKMATCHER);
}

int32_t
MockMatcher_next(MockMatcher* self) 
{
    if (++self->tick >= (int32_t)self->size) {
        self->tick--;
        return 0;
    }
    return I32Arr_Get(self->doc_ids, self->tick);
}

float
MockMatcher_score(MockMatcher* self) 
{
    if (!self->scores) { 
        THROW(ERR, "Can't call Score() unless scores supplied");
    }
    float *raw_scores = (float*)BB_Get_Buf(self->scores);
    return raw_scores[self->tick];
}

int32_t 
MockMatcher_get_doc_id(MockMatcher* self) 
{
    return I32Arr_Get(self->doc_ids, self->tick);
}


