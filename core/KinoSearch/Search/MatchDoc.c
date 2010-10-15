#define C_KINO_MATCHDOC
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/MatchDoc.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"

MatchDoc*
MatchDoc_new(int32_t doc_id, float score, VArray *values)
{
    MatchDoc *self = (MatchDoc*)VTable_Make_Obj(MATCHDOC);
    return MatchDoc_init(self, doc_id, score, values);
}

MatchDoc*
MatchDoc_init(MatchDoc *self, int32_t doc_id, float score, VArray *values)
{
    self->doc_id      = doc_id;
    self->score       = score;
    self->values      = (VArray*)INCREF(values);
    return self;
}

void
MatchDoc_destroy(MatchDoc *self)
{
    DECREF(self->values);
    SUPER_DESTROY(self, MATCHDOC);
}

void
MatchDoc_serialize(MatchDoc *self, OutStream *outstream)
{
    OutStream_Write_C32(outstream, self->doc_id);
    OutStream_Write_F32(outstream, self->score);
    OutStream_Write_U8(outstream, self->values ? 1 : 0);
    if (self->values) { VA_Serialize(self->values, outstream); }
}

MatchDoc*
MatchDoc_deserialize(MatchDoc *self, InStream *instream)
{
    self = self ? self : (MatchDoc*)VTable_Make_Obj(MATCHDOC);
    self->doc_id = InStream_Read_C32(instream);
    self->score  = InStream_Read_F32(instream);
    if (InStream_Read_U8(instream)) {
        self->values = VA_deserialize(NULL, instream);
    }
    return self;
}

int32_t
MatchDoc_get_doc_id(MatchDoc *self) { return self->doc_id; }
float
MatchDoc_get_score(MatchDoc *self)  { return self->score; }
VArray*
MatchDoc_get_values(MatchDoc *self) { return self->values; }
void
MatchDoc_set_doc_id(MatchDoc *self, int32_t doc_id) 
    { self->doc_id = doc_id; }
void
MatchDoc_set_score(MatchDoc *self, float score) 
    { self->score = score; }
void
MatchDoc_set_values(MatchDoc *self, VArray *values)
{
    DECREF(self->values);
    self->values = (VArray*)INCREF(values);
}


