#define C_KINO_FIELDTYPE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Plan/FieldType.h"
#include "KinoSearch/Analysis/Analyzer.h"
#include "KinoSearch/Index/Posting.h"
#include "KinoSearch/Index/Similarity.h"

FieldType*
FType_init(FieldType *self)
{
    return FType_init2(self, 1.0f, false, false, false);
}

FieldType*
FType_init2(FieldType *self, float boost, bool_t indexed, bool_t stored,
            bool_t sortable)
{
    self->boost              = boost;
    self->indexed            = indexed;
    self->stored             = stored;
    self->sortable           = sortable;
    ABSTRACT_CLASS_CHECK(self, FIELDTYPE);
    return self;
}

void
FType_set_boost(FieldType *self, float boost) 
    { self->boost = boost; }
void
FType_set_indexed(FieldType *self, bool_t indexed) 
    { self->indexed = !!indexed; }
void
FType_set_stored(FieldType *self, bool_t stored) 
    { self->stored = !!stored; }
void
FType_set_sortable(FieldType *self, bool_t sortable) 
    { self->sortable = !!sortable; }

float
FType_get_boost(FieldType *self)  { return self->boost; }
bool_t
FType_indexed(FieldType *self)    { return self->indexed; }
bool_t
FType_stored(FieldType *self)     { return self->stored; }
bool_t
FType_sortable(FieldType *self)   { return self->sortable; }

bool_t
FType_binary(FieldType *self) { 
    UNUSED_VAR(self); 
    return false; 
}

Similarity*
FType_similarity(FieldType *self)
{
    UNUSED_VAR(self);
    return NULL;
}

int32_t
FType_compare_values(FieldType *self, Obj *a, Obj *b)
{
    UNUSED_VAR(self);
    return Obj_Compare_To(a, b);
}

bool_t
FType_equals(FieldType *self, Obj *other)
{
    FieldType *evil_twin = (FieldType*)other;
    if (evil_twin == self) return true;
    if (self->boost != evil_twin->boost) return false;
    if (!!self->indexed    != !!evil_twin->indexed)    return false;
    if (!!self->stored     != !!evil_twin->stored)     return false;
    if (!!self->sortable   != !!evil_twin->sortable)   return false;
    if (!!FType_Binary(self) != !!FType_Binary(evil_twin)) return false;
    return true;
}


