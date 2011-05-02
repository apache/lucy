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

#define C_LUCY_COMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Compiler.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Search/Query.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

Compiler*
Compiler_init(Compiler *self, Query *parent, Searcher *searcher,
              Similarity *sim, float boost) {
    Query_init((Query*)self, boost);
    if (!sim) {
        Schema *schema = Searcher_Get_Schema(searcher);
        sim = Schema_Get_Similarity(schema);
    }
    self->parent  = (Query*)INCREF(parent);
    self->sim     = (Similarity*)INCREF(sim);
    ABSTRACT_CLASS_CHECK(self, COMPILER);
    return self;
}

void
Compiler_destroy(Compiler *self) {
    DECREF(self->parent);
    DECREF(self->sim);
    SUPER_DESTROY(self, COMPILER);
}

float
Compiler_get_weight(Compiler *self) {
    return Compiler_Get_Boost(self);
}

Similarity*
Compiler_get_similarity(Compiler *self) {
    return self->sim;
}

Query*
Compiler_get_parent(Compiler *self) {
    return self->parent;
}

float
Compiler_sum_of_squared_weights(Compiler *self) {
    UNUSED_VAR(self);
    return 1.0f;
}

void
Compiler_apply_norm_factor(Compiler *self, float factor) {
    UNUSED_VAR(self);
    UNUSED_VAR(factor);
}

void
Compiler_normalize(Compiler *self) {
    // factor = (tf_q * idf_t)
    float factor = Compiler_Sum_Of_Squared_Weights(self);

    // factor /= norm_q
    factor = Sim_Query_Norm(self->sim, factor);

    // weight *= factor
    Compiler_Apply_Norm_Factor(self, factor);
}

VArray*
Compiler_highlight_spans(Compiler *self, Searcher *searcher,
                         DocVector *doc_vec, const CharBuf *field) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(doc_vec);
    UNUSED_VAR(field);
    return VA_new(0);
}

CharBuf*
Compiler_to_string(Compiler *self) {
    CharBuf *stringified_query = Query_To_String(self->parent);
    CharBuf *string = CB_new_from_trusted_utf8("compiler(", 9);
    CB_Cat(string, stringified_query);
    CB_Cat_Trusted_Str(string, ")", 1);
    DECREF(stringified_query);
    return string;
}

bool_t
Compiler_equals(Compiler *self, Obj *other) {
    Compiler *twin = (Compiler*)other;
    if (twin == self)                                    { return true; }
    if (!Obj_Is_A(other, COMPILER))                      { return false; }
    if (self->boost != twin->boost)                      { return false; }
    if (!Query_Equals(self->parent, (Obj*)twin->parent)) { return false; }
    if (!Sim_Equals(self->sim, (Obj*)twin->sim))         { return false; }
    return true;
}

void
Compiler_serialize(Compiler *self, OutStream *outstream) {
    ABSTRACT_CLASS_CHECK(self, COMPILER);
    OutStream_Write_F32(outstream, self->boost);
    FREEZE(self->parent, outstream);
    FREEZE(self->sim, outstream);
}

Compiler*
Compiler_deserialize(Compiler *self, InStream *instream) {
    if (!self) { THROW(ERR, "Compiler_Deserialize is abstract"); }
    self->boost  = InStream_Read_F32(instream);
    self->parent = (Query*)THAW(instream);
    self->sim    = (Similarity*)THAW(instream);
    return self;
}


