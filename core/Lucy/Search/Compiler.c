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

#include "Clownfish/CharBuf.h"
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
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    Query_init((Query*)self, boost);
    if (!sim) {
        Schema *schema = Searcher_Get_Schema(searcher);
        sim = Schema_Get_Similarity(schema);
    }
    ivars->parent  = (Query*)INCREF(parent);
    ivars->sim     = (Similarity*)INCREF(sim);
    ABSTRACT_CLASS_CHECK(self, COMPILER);
    return self;
}

void
Compiler_Destroy_IMP(Compiler *self) {
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    DECREF(ivars->parent);
    DECREF(ivars->sim);
    SUPER_DESTROY(self, COMPILER);
}

float
Compiler_Get_Weight_IMP(Compiler *self) {
    return Compiler_Get_Boost(self);
}

Similarity*
Compiler_Get_Similarity_IMP(Compiler *self) {
    return Compiler_IVARS(self)->sim;
}

Query*
Compiler_Get_Parent_IMP(Compiler *self) {
    return Compiler_IVARS(self)->parent;
}

float
Compiler_Sum_Of_Squared_Weights_IMP(Compiler *self) {
    UNUSED_VAR(self);
    return 1.0f;
}

void
Compiler_Apply_Norm_Factor_IMP(Compiler *self, float factor) {
    UNUSED_VAR(self);
    UNUSED_VAR(factor);
}

void
Compiler_Normalize_IMP(Compiler *self) {
    CompilerIVARS *const ivars = Compiler_IVARS(self);

    // factor = (tf_q * idf_t)
    float factor = Compiler_Sum_Of_Squared_Weights(self);

    // factor /= norm_q
    factor = Sim_Query_Norm(ivars->sim, factor);

    // weight *= factor
    Compiler_Apply_Norm_Factor(self, factor);
}

VArray*
Compiler_Highlight_Spans_IMP(Compiler *self, Searcher *searcher,
                             DocVector *doc_vec, String *field) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(doc_vec);
    UNUSED_VAR(field);
    return VA_new(0);
}

String*
Compiler_To_String_IMP(Compiler *self) {
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    String *stringified_query = Query_To_String(ivars->parent);
    CharBuf *buf = CB_new_from_trusted_utf8("compiler(", 9);
    CB_Cat(buf, stringified_query);
    CB_Cat_Trusted_Utf8(buf, ")", 1);
    String *string = CB_Yield_String(buf);
    DECREF(buf);
    DECREF(stringified_query);
    return string;
}

bool
Compiler_Equals_IMP(Compiler *self, Obj *other) {
    if ((Compiler*)other == self)                          { return true; }
    if (!Obj_Is_A(other, COMPILER))                        { return false; }
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    CompilerIVARS *const ovars = Compiler_IVARS((Compiler*)other);
    if (ivars->boost != ovars->boost)                      { return false; }
    if (!Query_Equals(ivars->parent, (Obj*)ovars->parent)) { return false; }
    if (!Sim_Equals(ivars->sim, (Obj*)ovars->sim))         { return false; }
    return true;
}

void
Compiler_Serialize_IMP(Compiler *self, OutStream *outstream) {
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    ABSTRACT_CLASS_CHECK(self, COMPILER);
    OutStream_Write_F32(outstream, ivars->boost);
    FREEZE(ivars->parent, outstream);
    FREEZE(ivars->sim, outstream);
}

Compiler*
Compiler_Deserialize_IMP(Compiler *self, InStream *instream) {
    CompilerIVARS *const ivars = Compiler_IVARS(self);
    ivars->boost  = InStream_Read_F32(instream);
    ivars->parent = (Query*)THAW(instream);
    ivars->sim    = (Similarity*)THAW(instream);
    return self;
}


