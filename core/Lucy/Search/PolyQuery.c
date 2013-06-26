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
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    ivars->children = VA_new(num_kids);
    for (uint32_t i = 0; i < num_kids; i++) {
        PolyQuery_Add_Child(self, (Query*)VA_Fetch(children, i));
    }
    return self;
}

void
PolyQuery_destroy(PolyQuery *self) {
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    DECREF(ivars->children);
    SUPER_DESTROY(self, POLYQUERY);
}

void
PolyQuery_add_child(PolyQuery *self, Query *query) {
    CERTIFY(query, QUERY);
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    VA_Push(ivars->children, INCREF(query));
}

void
PolyQuery_set_children(PolyQuery *self, VArray *children) {
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    DECREF(ivars->children);
    ivars->children = (VArray*)INCREF(children);
}

VArray*
PolyQuery_get_children(PolyQuery *self) {
    return PolyQuery_IVARS(self)->children;
}

void
PolyQuery_serialize(PolyQuery *self, OutStream *outstream) {
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    const uint32_t num_kids = VA_Get_Size(ivars->children);
    OutStream_Write_F32(outstream, ivars->boost);
    OutStream_Write_U32(outstream, num_kids);
    for (uint32_t i = 0; i < num_kids; i++) {
        Query *child = (Query*)VA_Fetch(ivars->children, i);
        FREEZE(child, outstream);
    }
}

PolyQuery*
PolyQuery_deserialize(PolyQuery *self, InStream *instream) {
    float    boost        = InStream_Read_F32(instream);
    uint32_t num_children = InStream_Read_U32(instream);
    PolyQuery_init(self, NULL);
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    PolyQuery_Set_Boost(self, boost);
    VA_Grow(ivars->children, num_children);
    while (num_children--) {
        VA_Push(ivars->children, THAW(instream));
    }
    return self;
}

bool
PolyQuery_equals(PolyQuery *self, Obj *other) {
    if ((PolyQuery*)other == self)                          { return true; }
    if (!Obj_Is_A(other, POLYQUERY))                        { return false; }
    PolyQueryIVARS *const ivars = PolyQuery_IVARS(self);
    PolyQueryIVARS *const ovars = PolyQuery_IVARS((PolyQuery*)other);
    if (ivars->boost != ovars->boost)                       { return false; }
    if (!VA_Equals(ovars->children, (Obj*)ivars->children)) { return false; }
    return true;
}

/**********************************************************************/


PolyCompiler*
PolyCompiler_init(PolyCompiler *self, PolyQuery *parent,
                  Searcher *searcher, float boost) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    PolyQueryIVARS *const parent_ivars = PolyQuery_IVARS(parent);
    const uint32_t num_kids = VA_Get_Size(parent_ivars->children);

    Compiler_init((Compiler*)self, (Query*)parent, searcher, NULL, boost);
    ivars->children = VA_new(num_kids);

    // Iterate over the children, creating a Compiler for each one.
    for (uint32_t i = 0; i < num_kids; i++) {
        Query *child_query = (Query*)VA_Fetch(parent_ivars->children, i);
        float sub_boost = boost * Query_Get_Boost(child_query);
        Compiler *child_compiler
            = Query_Make_Compiler(child_query, searcher, sub_boost, true);
        VA_Push(ivars->children, (Obj*)child_compiler);
    }

    return self;
}

void
PolyCompiler_destroy(PolyCompiler *self) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    DECREF(ivars->children);
    SUPER_DESTROY(self, POLYCOMPILER);
}

float
PolyCompiler_sum_of_squared_weights(PolyCompiler *self) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    float sum      = 0;
    float my_boost = PolyCompiler_Get_Boost(self);

    for (uint32_t i = 0, max = VA_Get_Size(ivars->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(ivars->children, i);
        sum += Compiler_Sum_Of_Squared_Weights(child);
    }

    // Compound the weight of each child.
    sum *= my_boost * my_boost;

    return sum;
}

void
PolyCompiler_apply_norm_factor(PolyCompiler *self, float factor) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(ivars->children, i);
        Compiler_Apply_Norm_Factor(child, factor);
    }
}

VArray*
PolyCompiler_highlight_spans(PolyCompiler *self, Searcher *searcher,
                             DocVector *doc_vec, const CharBuf *field) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    VArray *spans = VA_new(0);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->children); i < max; i++) {
        Compiler *child = (Compiler*)VA_Fetch(ivars->children, i);
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
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    Freezer_serialize_charbuf(PolyCompiler_Get_Class_Name(self), outstream);
    Freezer_serialize_varray(ivars->children, outstream);
    PolyCompiler_Serialize_t super_serialize
        = SUPER_METHOD_PTR(POLYCOMPILER, Lucy_PolyCompiler_Serialize);
    super_serialize(self, outstream);
}

PolyCompiler*
PolyCompiler_deserialize(PolyCompiler *self, InStream *instream) {
    PolyCompilerIVARS *const ivars = PolyCompiler_IVARS(self);
    CharBuf *class_name = Freezer_read_charbuf(instream);
    DECREF(class_name); // TODO Don't serialize class name.
    ivars->children = Freezer_read_varray(instream);
    PolyCompiler_Deserialize_t super_deserialize
        = SUPER_METHOD_PTR(POLYCOMPILER, Lucy_PolyCompiler_Deserialize);
    return super_deserialize(self, instream);
}

