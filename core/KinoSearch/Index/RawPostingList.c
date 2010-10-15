#define C_KINO_RAWPOSTINGLIST
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/RawPostingList.h"
#include "KinoSearch/Index/Posting.h"
#include "KinoSearch/Index/Posting/RawPosting.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Util/MemoryPool.h"

RawPostingList*
RawPList_new(Schema *schema, const CharBuf *field, InStream *instream, 
             int64_t start, int64_t end)
{
    RawPostingList *self = (RawPostingList*)VTable_Make_Obj(RAWPOSTINGLIST);
    return RawPList_init(self, schema, field, instream, start, end);
}

RawPostingList*
RawPList_init(RawPostingList *self, Schema *schema, const CharBuf *field,
              InStream *instream, int64_t start, int64_t end)
{
    PList_init((PostingList*)self);
    self->start     = start;
    self->end       = end;
    self->len       = end - start;
    self->instream  = (InStream*)INCREF(instream);
    Similarity *sim = Schema_Fetch_Sim(schema, field);
    self->posting   = Sim_Make_Posting(sim);
    InStream_Seek(self->instream, self->start);
    return self;
}

void
RawPList_destroy(RawPostingList *self)
{
    DECREF(self->instream);
    DECREF(self->posting);
    SUPER_DESTROY(self, RAWPOSTINGLIST);
}

Posting*
RawPList_get_posting(RawPostingList *self)
{
    return self->posting;
}

RawPosting*
RawPList_read_raw(RawPostingList *self, int32_t last_doc_id, CharBuf *term_text,
                  MemoryPool *mem_pool)
{
    return Post_Read_Raw(self->posting, self->instream, 
        last_doc_id, term_text, mem_pool);
}


