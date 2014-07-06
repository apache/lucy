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

#define C_LUCY_TERMQUERY
#define C_LUCY_TERMCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/TermQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/TermVector.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Compiler.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Search/TermMatcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

TermQuery*
TermQuery_new(String *field, Obj *term) {
    TermQuery *self = (TermQuery*)Class_Make_Obj(TERMQUERY);
    return TermQuery_init(self, field, term);
}

TermQuery*
TermQuery_init(TermQuery *self, String *field, Obj *term) {
    Query_init((Query*)self, 1.0f);
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    ivars->field  = Str_Clone(field);
    ivars->term   = Obj_Clone(term);
    return self;
}

void
TermQuery_Destroy_IMP(TermQuery *self) {
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->term);
    SUPER_DESTROY(self, TERMQUERY);
}

void
TermQuery_Serialize_IMP(TermQuery *self, OutStream *outstream) {
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    Freezer_serialize_string(ivars->field, outstream);
    FREEZE(ivars->term, outstream);
    OutStream_Write_F32(outstream, ivars->boost);
}

TermQuery*
TermQuery_Deserialize_IMP(TermQuery *self, InStream *instream) {
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    ivars->field = Freezer_read_string(instream);
    ivars->term  = (Obj*)THAW(instream);
    ivars->boost = InStream_Read_F32(instream);
    return self;
}

Obj*
TermQuery_Dump_IMP(TermQuery *self) {
    TermQueryIVARS *ivars = TermQuery_IVARS(self);
    TermQuery_Dump_t super_dump
        = SUPER_METHOD_PTR(TERMQUERY, LUCY_TermQuery_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Utf8(dump, "field", 5, Freezer_dump((Obj*)ivars->field));
    Hash_Store_Utf8(dump, "term", 4, Freezer_dump(ivars->term));
    return (Obj*)dump;
}

Obj*
TermQuery_Load_IMP(TermQuery *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    TermQuery_Load_t super_load
        = SUPER_METHOD_PTR(TERMQUERY, LUCY_TermQuery_Load);
    TermQuery *loaded = (TermQuery*)super_load(self, dump);
    TermQueryIVARS *loaded_ivars = TermQuery_IVARS(loaded);
    Obj *field = CERTIFY(Hash_Fetch_Utf8(source, "field", 5), OBJ);
    loaded_ivars->field = (String*)CERTIFY(Freezer_load(field), STRING);
    Obj *term = CERTIFY(Hash_Fetch_Utf8(source, "term", 4), OBJ);
    loaded_ivars->term = (Obj*)CERTIFY(Freezer_load(term), OBJ);
    return (Obj*)loaded;
}

String*
TermQuery_Get_Field_IMP(TermQuery *self) {
    return TermQuery_IVARS(self)->field;
}

Obj*
TermQuery_Get_Term_IMP(TermQuery *self) {
    return TermQuery_IVARS(self)->term;
}

bool
TermQuery_Equals_IMP(TermQuery *self, Obj *other) {
    if ((TermQuery*)other == self)                     { return true; }
    if (!Obj_Is_A(other, TERMQUERY))                   { return false; }
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    TermQueryIVARS *const ovars = TermQuery_IVARS((TermQuery*)other);
    if (ivars->boost != ovars->boost)                  { return false; }
    if (!Str_Equals(ivars->field, (Obj*)ovars->field)) { return false; }
    if (!Obj_Equals(ivars->term, ovars->term))         { return false; }
    return true;
}

String*
TermQuery_To_String_IMP(TermQuery *self) {
    TermQueryIVARS *const ivars = TermQuery_IVARS(self);
    String *term_str = Obj_To_String(ivars->term);
    String *retval = Str_newf("%o:%o", ivars->field, term_str);
    DECREF(term_str);
    return retval;
}

Compiler*
TermQuery_Make_Compiler_IMP(TermQuery *self, Searcher *searcher, float boost,
                            bool subordinate) {
    TermCompiler *compiler = TermCompiler_new((Query*)self, searcher, boost);
    if (!subordinate) {
        TermCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;

}

/******************************************************************/

TermCompiler*
TermCompiler_new(Query *parent, Searcher *searcher, float boost) {
    TermCompiler *self = (TermCompiler*)Class_Make_Obj(TERMCOMPILER);
    return TermCompiler_init(self, parent, searcher, boost);
}

TermCompiler*
TermCompiler_init(TermCompiler *self, Query *parent, Searcher *searcher,
                  float boost) {
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    TermQueryIVARS *const parent_ivars = TermQuery_IVARS((TermQuery*)parent);
    Schema     *schema  = Searcher_Get_Schema(searcher);
    Similarity *sim     = Schema_Fetch_Sim(schema, parent_ivars->field);

    // Try harder to get a Similarity if necessary.
    if (!sim) { sim = Schema_Get_Similarity(schema); }

    // Init.
    Compiler_init((Compiler*)self, parent, searcher, sim, boost);
    ivars->normalized_weight = 0.0f;
    ivars->query_norm_factor = 0.0f;

    // Derive.
    int32_t doc_max  = Searcher_Doc_Max(searcher);
    int32_t doc_freq = Searcher_Doc_Freq(searcher, parent_ivars->field,
                                         parent_ivars->term);
    ivars->idf = Sim_IDF(sim, doc_freq, doc_max);

    /* The score of any document is approximately equal to:
     *
     *    (tf_d * idf_t / norm_d) * (tf_q * idf_t / norm_q)
     *
     * Here we add in the first IDF, plus user-supplied boost.
     *
     * The second clause is factored in by the call to Normalize().
     *
     * tf_d and norm_d can only be added by the Matcher, since they are
     * per-document.
     */
    ivars->raw_weight = ivars->idf * ivars->boost;

    return self;
}

bool
TermCompiler_Equals_IMP(TermCompiler *self, Obj *other) {
    TermCompiler_Equals_t super_equals
        = (TermCompiler_Equals_t)SUPER_METHOD_PTR(TERMCOMPILER,
                                                  LUCY_TermCompiler_Equals);
    if (!super_equals(self, other))                           { return false; }
    if (!Obj_Is_A(other, TERMCOMPILER))                       { return false; }
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    TermCompilerIVARS *const ovars = TermCompiler_IVARS((TermCompiler*)other);
    if (ivars->idf != ovars->idf)                             { return false; }
    if (ivars->raw_weight != ovars->raw_weight)               { return false; }
    if (ivars->query_norm_factor != ovars->query_norm_factor) { return false; }
    if (ivars->normalized_weight != ovars->normalized_weight) { return false; }
    return true;
}

void
TermCompiler_Serialize_IMP(TermCompiler *self, OutStream *outstream) {
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    TermCompiler_Serialize_t super_serialize
        = SUPER_METHOD_PTR(TERMCOMPILER, LUCY_TermCompiler_Serialize);
    super_serialize(self, outstream);
    OutStream_Write_F32(outstream, ivars->idf);
    OutStream_Write_F32(outstream, ivars->raw_weight);
    OutStream_Write_F32(outstream, ivars->query_norm_factor);
    OutStream_Write_F32(outstream, ivars->normalized_weight);
}

TermCompiler*
TermCompiler_Deserialize_IMP(TermCompiler *self, InStream *instream) {
    TermCompiler_Deserialize_t super_deserialize
        = SUPER_METHOD_PTR(TERMCOMPILER, LUCY_TermCompiler_Deserialize);
    self = super_deserialize(self, instream);
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    ivars->idf               = InStream_Read_F32(instream);
    ivars->raw_weight        = InStream_Read_F32(instream);
    ivars->query_norm_factor = InStream_Read_F32(instream);
    ivars->normalized_weight = InStream_Read_F32(instream);
    return self;
}

float
TermCompiler_Sum_Of_Squared_Weights_IMP(TermCompiler *self) {
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    return ivars->raw_weight * ivars->raw_weight;
}

void
TermCompiler_Apply_Norm_Factor_IMP(TermCompiler *self,
                                   float query_norm_factor) {
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    ivars->query_norm_factor = query_norm_factor;

    /* Multiply raw weight by the idf and norm_q factors in this:
     *
     *      (tf_q * idf_q / norm_q)
     *
     * Note: factoring in IDF a second time is correct.  See formula.
     */
    ivars->normalized_weight
        = ivars->raw_weight * ivars->idf * query_norm_factor;
}

float
TermCompiler_Get_Weight_IMP(TermCompiler *self) {
    return TermCompiler_IVARS(self)->normalized_weight;
}

Matcher*
TermCompiler_Make_Matcher_IMP(TermCompiler *self, SegReader *reader,
                              bool need_score) {
    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    TermQueryIVARS *const parent_ivars
        = TermQuery_IVARS((TermQuery*)ivars->parent);
    PostingListReader *plist_reader
        = (PostingListReader*)SegReader_Fetch(
              reader, Class_Get_Name(POSTINGLISTREADER));
    PostingList *plist = plist_reader
                         ? PListReader_Posting_List(plist_reader,
                                                    parent_ivars->field,
                                                    parent_ivars->term)
                         : NULL;

    if (plist == NULL || PList_Get_Doc_Freq(plist) == 0) {
        DECREF(plist);
        return NULL;
    }
    else {
        Matcher *retval = PList_Make_Matcher(plist, ivars->sim,
                                             (Compiler*)self, need_score);
        DECREF(plist);
        return retval;
    }
}

VArray*
TermCompiler_Highlight_Spans_IMP(TermCompiler *self, Searcher *searcher,
                                 DocVector *doc_vec, String *field) {

    TermCompilerIVARS *const ivars = TermCompiler_IVARS(self);
    TermQueryIVARS *const parent_ivars
        = TermQuery_IVARS((TermQuery*)ivars->parent);
    VArray *spans = VA_new(0);
    TermVector *term_vector;
    I32Array *starts, *ends;
    UNUSED_VAR(searcher);

    if (!Str_Equals(parent_ivars->field, (Obj*)field)) { return spans; }

    // Add all starts and ends.
    term_vector
        = DocVec_Term_Vector(doc_vec, field, (String*)parent_ivars->term);
    if (!term_vector) { return spans; }

    starts = TV_Get_Start_Offsets(term_vector);
    ends   = TV_Get_End_Offsets(term_vector);
    for (uint32_t i = 0, max = I32Arr_Get_Size(starts); i < max; i++) {
        int32_t start  = I32Arr_Get(starts, i);
        int32_t length = I32Arr_Get(ends, i) - start;
        VA_Push(spans,
                (Obj*)Span_new(start, length, TermCompiler_Get_Weight(self)));
    }

    DECREF(term_vector);
    return spans;
}


