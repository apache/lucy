#define C_LUCY_FIELDTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/FieldType.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/Similarity/LuceneSimilarity.h"

FieldType*
FType_init(FieldType *self, Similarity *similarity)
{
    return FType_init2(self, similarity, 1.0f, false, false, false);
}

FieldType*
FType_init2(FieldType *self, Similarity *similarity, float boost, 
            bool_t indexed, bool_t stored, bool_t sortable)
{
    self->sim = (Similarity*)INCREF(similarity);
    self->sim = similarity ? (Similarity*)INCREF(similarity) 
              : indexed    ? (Similarity*)LuceneSim_new()
              : NULL;
    
    if (indexed && !self->sim) {
        THROW(ERR, "Indexed FieldType missing Similarity");
    }
    self->boost              = boost;
    self->indexed            = indexed;
    self->stored             = stored;
    self->sortable           = sortable;
    ABSTRACT_CLASS_CHECK(self, FIELDTYPE);
    return self;
}

void
FType_destroy(FieldType *self)
{
    DECREF(self->sim);
    SUPER_DESTROY(self, FIELDTYPE);
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
Similarity*
FType_get_similarity(FieldType *self) { return self->sim; }

bool_t
FType_binary(FieldType *self) { 
    UNUSED_VAR(self); 
    return false; 
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
    if (!other) return false;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, FIELDTYPE)) return false;
    if (self->boost != evil_twin->boost) return false;
    if (!!self->indexed    != !!evil_twin->indexed)    return false;
    if (!!self->stored     != !!evil_twin->stored)     return false;
    if (!!self->sortable   != !!evil_twin->sortable)   return false;
    if (!!self->sim        != !!evil_twin->sim)        return false;
    if (!!FType_Binary(self) != !!FType_Binary(evil_twin)) return false;
    if (self->sim) {
        if (!Sim_Equals(self->sim, (Obj*)evil_twin->sim)) return false;
    }
    return true;
}

/* Copyright 2010 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

