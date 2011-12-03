/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define C_LUCY_POLYQUERY
#define C_LUCY_POLYCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/PolyQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

PolyQuery*
PolyQuery_init(PolyQuery *self, VArray *children) {
    const uint32_t num_kids = children ? VA_Get_Size(children) : 0;
    Query_init((Query*)self, 1.0f);
    self->children = VA_new(num_kids);
    for (uint32_t i = 0; i < num_kids; i++) {
        PolyQuery_Add_Child(self, (Query*)VA_Fetch(children, i));
    }
    return self;
}

void
PolyQuery_destroy(PolyQuery *self) {
    DECREF(self->children);
    SUPER_DESTROY(self, POLYQUERY);
}

void
PolyQuery_add_child(PolyQuery *self, Query *query) {
    CERTIFY(query, QUERY);
    VA_Push(self->children, INCREF(query));
}

void
PolyQuery_set_children(PolyQuery *self, VArray *children) {
    DECREF(self->children);
    self->children = (VArray*)INCREF(children);
}

VArray*
PolyQuery_get_children(PolyQuery *self) {
    return self->children;
}

void
PolyQuery_serialize(PolyQuery *self, OutStream *outstream) {
    const uint32_t num_kids = VA_Get_Size(self->children);
    OutStream_Write_F32(outstream, self->boost);
    OutStream_Write_U32(outstream, num_kids);
    for (uint32_t i = 0; i < num_kids; i++) {
        Query *child = (Query*)VA_Fetch(self->children, i);
        FREEZE(child, outstream);
    }
}

PolyQuery*
PolyQuery_deserialize(PolyQuery *self, InStream *instream) {
    float    boost        = InStream_Read_F32(instream);
    uint32_t num_children = InStream_Read_U32(instream);

    if (!self) { THROW(ERR, "Abstract class"); }
    PolyQuery_init(self, NULL);
    PolyQuery_Set_Boost(self, boost);

    VA_Grow(self->children, num_children);
    while (num_children--) {
        VA_Push(self->children, THAW(instream));
    }

    return self;
}

bool_t
PolyQuery_equals(PolyQuery *self, Obj *other) {
    PolyQuery *twin = (PolyQuery*)other;
    if (twin == self)                                     { return true; }
    if (!Obj_Is_A(other, POLYQUERY))                      { return false; }
    if (self->boost != twin->boost)                       { return false; }
    if (!VA_Equals(twin->children, (Obj*)self->children)) { return false; }
    return true;
}

/**********************************************************************/


PolyCompiler*
PolyCompiler_init(PolyCompiler *self, PolyQuery *parent,
                  Searcher *searcher, float boost) {
    const uint32_t num_kids = VA_Get_Size(parent->children);

    Compiler_init((Compiler*)self, (Query*)parent, searcher, NULL, boost);
    self->children = VA_new(num_kids);

    // Iterate over the children, creating a Compiler for each one.
    for (uint32_t i = 0; i < num_kids; i++) {
        Query *child_query = (Query*)VA_Fetch(parent->children, i);
        float sub_boost = boost * Query_Get_Boost(child_query);
        Compiler *child_compiler
            = Query_Make_Compiler(child_query, searcher, sub_boost, true);
        VA_Push(self->children, (Obj*)child_compiler);
    }

    return self;
}

void
PolyCompiler_destroy(PolyCompiler *self) {
    DECREF(self->children);
    SUPER_DESTROY(self, POLYCOMPILER);
}

float
PolyCompiler_sum_of_squared_weights(PolyCompiler *self) {
    float sum      = 0;
    float my_boost = PolyCompiler_Get_Boost(self);

    for (uint32_t i = 0, max = VA_Get_Size(self->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(self->children, i);
        sum += Compiler_Sum_Of_Squared_Weights(child);
    }

    // Compound the weight of each child.
    sum *= my_boost * my_boost;

    return sum;
}

void
PolyCompiler_apply_norm_factor(PolyCompiler *self, float factor) {
    for (uint32_t i = 0, max = VA_Get_Size(self->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(self->children, i);
        Compiler_Apply_Norm_Factor(child, factor);
    }
}

VArray*
PolyCompiler_highlight_spans(PolyCompiler *self, Searcher *searcher,
                             DocVector *doc_vec, const CharBuf *field) {
    VArray *spans = VA_new(0);
    for (uint32_t i = 0, max = VA_Get_Size(self->children); i < max; i++) {
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
PolyCompiler_serialize(PolyCompiler *self, OutStream *outstream) {
    CB_Serialize(PolyCompiler_Get_Class_Name(self), outstream);
    VA_Serialize(self->children, outstream);
    Compiler_serialize((Compiler*)self, outstream);
}

PolyCompiler*
PolyCompiler_deserialize(PolyCompiler *self, InStream *instream) {
    CharBuf *class_name = CB_deserialize(NULL, instream);
    if (!self) {
        VTable *vtable = VTable_singleton(class_name, NULL);
        self = (PolyCompiler*)VTable_Make_Obj(vtable);
    }
    DECREF(class_name);
    self->children = VA_deserialize(NULL, instream);
    return (PolyCompiler*)Compiler_deserialize((Compiler*)self, instream);
}


