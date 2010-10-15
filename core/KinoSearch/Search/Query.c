#define C_KINO_QUERY
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/Query.h"
#include "KinoSearch/Search/Compiler.h"
#include "KinoSearch/Search/Searcher.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"

Query*
Query_init(Query *self, float boost)
{
    self->boost = boost;
    ABSTRACT_CLASS_CHECK(self, QUERY);
    return self;
}

void
Query_set_boost(Query *self, float boost) { self->boost = boost; }
float
Query_get_boost(Query *self)              { return self->boost; }

void
Query_serialize(Query *self, OutStream *outstream)
{
    OutStream_Write_F32(outstream, self->boost);
}

Query*
Query_deserialize(Query *self, InStream *instream)
{
    float boost = InStream_Read_F32(instream);
    self = self ? self : (Query*)VTable_Make_Obj(QUERY);
    Query_init(self, boost);
    return self;
}


