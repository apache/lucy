#define C_KINO_TOPDOCS
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/TopDocs.h"
#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/Lexicon.h"
#include "KinoSearch/Search/SortRule.h"
#include "KinoSearch/Search/SortSpec.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"

TopDocs*
TopDocs_new(VArray *match_docs, uint32_t total_hits)
{
    TopDocs *self = (TopDocs*)VTable_Make_Obj(TOPDOCS);
    return TopDocs_init(self, match_docs, total_hits);
}

TopDocs*
TopDocs_init(TopDocs *self, VArray *match_docs, uint32_t total_hits)
{
    self->match_docs = (VArray*)INCREF(match_docs);
    self->total_hits = total_hits;
    return self;
}

void
TopDocs_destroy(TopDocs *self)
{
    DECREF(self->match_docs);
    SUPER_DESTROY(self, TOPDOCS);
}

void
TopDocs_serialize(TopDocs *self, OutStream *outstream)
{
    VA_Serialize(self->match_docs, outstream);
    OutStream_Write_C32(outstream, self->total_hits);
}

TopDocs*
TopDocs_deserialize(TopDocs *self, InStream *instream)
{
    self = self ? self : (TopDocs*)VTable_Make_Obj(TOPDOCS);
    self->match_docs = VA_deserialize(NULL, instream);
    self->total_hits = InStream_Read_C32(instream);
    return self;
}

VArray*
TopDocs_get_match_docs(TopDocs *self) { return self->match_docs; }
uint32_t
TopDocs_get_total_hits(TopDocs *self) { return self->total_hits; }

void
TopDocs_set_match_docs(TopDocs *self, VArray *match_docs)
{
    DECREF(self->match_docs);
    self->match_docs = (VArray*)INCREF(match_docs);
}
void
TopDocs_set_total_hits(TopDocs *self, uint32_t total_hits) 
    { self->total_hits = total_hits; }


