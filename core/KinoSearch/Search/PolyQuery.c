#define C_KINO_POLYQUERY
#define C_KINO_POLYCOMPILER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/PolyQuery.h"
#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/Searcher.h"
#include "KinoSearch/Search/Span.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/Freezer.h"

PolyQuery*
PolyQuery_init(PolyQuery *self, VArray *children)
{
    uint32_t i;
    const uint32_t num_kids = children ? VA_Get_Size(children) : 0;
    Query_init((Query*)self, 1.0f);
    self->children = VA_new(num_kids);
    for (i = 0; i < num_kids; i++) {
        PolyQuery_Add_Child(self, (Query*)VA_Fetch(children, i));
    }
    return self;
}

void
PolyQuery_destroy(PolyQuery *self)
{
    DECREF(self->children);
    SUPER_DESTROY(self, POLYQUERY);
}

void
PolyQuery_add_child(PolyQuery *self, Query *query)
{
    CERTIFY(query, QUERY);
    VA_Push(self->children, INCREF(query));
}

void
PolyQuery_set_children(PolyQuery *self, VArray *children)
{
    DECREF(self->children);
    self->children = (VArray*)INCREF(children);
}

VArray*
PolyQuery_get_children(PolyQuery *self) { return self->children; }

void
PolyQuery_serialize(PolyQuery *self, OutStream *outstream)
{
    const uint32_t num_kids =  VA_Get_Size(self->children);
    uint32_t i;
    OutStream_Write_F32(outstream, self->boost);
    OutStream_Write_U32(outstream, num_kids);
    for (i = 0; i < num_kids; i++) {
        Query *child = (Query*)VA_Fetch(self->children, i);
        FREEZE(child, outstream);
    }
}

PolyQuery*
PolyQuery_deserialize(PolyQuery *self, InStream *instream)
{
    float boost          = InStream_Read_F32(instream);
    uint32_t num_children   = InStream_Read_U32(instream);

    if (!self) THROW(ERR, "Abstract class");
    PolyQuery_init(self, NULL);
    PolyQuery_Set_Boost(self, boost);

    VA_Grow(self->children, num_children);
    while (num_children--) {
        VA_Push(self->children, THAW(instream));
    }

    return self;
}

bool_t
PolyQuery_equals(PolyQuery *self, Obj *other)
{
    PolyQuery *evil_twin = (PolyQuery*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, POLYQUERY)) return false;
    if (self->boost != evil_twin->boost) return false;
    if (!VA_Equals(evil_twin->children, (Obj*)self->children)) return false;
    return true;
}

/**********************************************************************/


PolyCompiler*
PolyCompiler_init(PolyCompiler *self, PolyQuery *parent, 
                  Searcher *searcher, float boost)
{
    uint32_t i;
    const uint32_t num_kids = VA_Get_Size(parent->children);

    Compiler_init((Compiler*)self, (Query*)parent, searcher, NULL, boost);
    self->children = VA_new(num_kids);

    // Iterate over the children, creating a Compiler for each one. 
    for (i = 0; i < num_kids; i++) {
        Query *child_query = (Query*)VA_Fetch(parent->children, i);
        float sub_boost = boost * Query_Get_Boost(child_query);
        VA_Push(self->children, 
            (Obj*)Query_Make_Compiler(child_query, searcher, sub_boost));
    }

    return self;
}

void
PolyCompiler_destroy(PolyCompiler *self)
{
    DECREF(self->children);
    SUPER_DESTROY(self, POLYCOMPILER);
}

float
PolyCompiler_sum_of_squared_weights(PolyCompiler *self)
{
    float sum = 0;
    uint32_t i, max;
    float my_boost = PolyCompiler_Get_Boost(self);

    for (i = 0, max = VA_Get_Size(self->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(self->children, i);
        sum += Compiler_Sum_Of_Squared_Weights(child);
    }

    // Compound the weight of each child. 
    sum *= my_boost * my_boost;

    return sum;
}

void
PolyCompiler_apply_norm_factor(PolyCompiler *self, float factor)
{
    uint32_t i, max;
    for (i = 0, max = VA_Get_Size(self->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(self->children, i);
        Compiler_Apply_Norm_Factor(child, factor);
    }
}

VArray*
PolyCompiler_highlight_spans(PolyCompiler *self, Searcher *searcher, 
                            DocVector *doc_vec, const CharBuf *field)
{
    VArray *spans = VA_new(0);
    uint32_t i, max;
    for (i = 0, max = VA_Get_Size(self->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(self->children, i);
        VArray *child_spans = Compiler_Highlight_Spans(child, searcher,
            doc_vec, field);
        if (child_spans) {
            VA_Push_VArray(spans, child_spans);
            VA_Dec_RefCount(child_spans);
        }
    }
    return spans;
}

void
PolyCompiler_serialize(PolyCompiler *self, OutStream *outstream)
{
    CB_Serialize(PolyCompiler_Get_Class_Name(self), outstream);
    VA_Serialize(self->children, outstream);
    Compiler_serialize((Compiler*)self, outstream);
}

PolyCompiler*
PolyCompiler_deserialize(PolyCompiler *self, InStream *instream)
{
    CharBuf *class_name = CB_deserialize(NULL, instream);
    if (!self) {
        VTable *vtable = VTable_singleton(class_name, NULL);
        self = (PolyCompiler*)VTable_Make_Obj(vtable);
    }
    DECREF(class_name);
    self->children = VA_deserialize(NULL, instream);
    return (PolyCompiler*)Compiler_deserialize((Compiler*)self, instream);
}


