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
TermQuery_new(const CharBuf *field, const Obj *term) {
    TermQuery *self = (TermQuery*)VTable_Make_Obj(TERMQUERY);
    return TermQuery_init(self, field, term);
}

TermQuery*
TermQuery_init(TermQuery *self, const CharBuf *field, const Obj *term) {
    Query_init((Query*)self, 1.0f);
    self->field       = CB_Clone(field);
    self->term        = Obj_Clone(term);
    return self;
}

void
TermQuery_destroy(TermQuery *self) {
    DECREF(self->field);
    DECREF(self->term);
    SUPER_DESTROY(self, TERMQUERY);
}

void
TermQuery_serialize(TermQuery *self, OutStream *outstream) {
    CB_Serialize(self->field, outstream);
    FREEZE(self->term, outstream);
    OutStream_Write_F32(outstream, self->boost);
}

TermQuery*
TermQuery_deserialize(TermQuery *self, InStream *instream) {
    self = self ? self : (TermQuery*)VTable_Make_Obj(TERMQUERY);
    self->field = CB_deserialize(NULL, instream);
    self->term  = (Obj*)THAW(instream);
    self->boost = InStream_Read_F32(instream);
    return self;
}

CharBuf*
TermQuery_get_field(TermQuery *self) {
    return self->field;
}

Obj*
TermQuery_get_term(TermQuery *self) {
    return self->term;
}

bool_t
TermQuery_equals(TermQuery *self, Obj *other) {
    TermQuery *twin = (TermQuery*)other;
    if (twin == self)                               { return true; }
    if (!Obj_Is_A(other, TERMQUERY))                { return false; }
    if (self->boost != twin->boost)                 { return false; }
    if (!CB_Equals(self->field, (Obj*)twin->field)) { return false; }
    if (!Obj_Equals(self->term, twin->term))        { return false; }
    return true;
}

CharBuf*
TermQuery_to_string(TermQuery *self) {
    CharBuf *term_str = Obj_To_String(self->term);
    CharBuf *retval = CB_newf("%o:%o", self->field, term_str);
    DECREF(term_str);
    return retval;
}

Compiler*
TermQuery_make_compiler(TermQuery *self, Searcher *searcher, float boost,
                        bool_t subordinate) {
    TermCompiler *compiler = TermCompiler_new((Query*)self, searcher, boost);
    if (!subordinate) {
        TermCompiler_Normalize(compiler);
    }   
    return (Compiler*)compiler;

}

/******************************************************************/

TermCompiler*
TermCompiler_new(Query *parent, Searcher *searcher, float boost) {
    TermCompiler *self = (TermCompiler*)VTable_Make_Obj(TERMCOMPILER);
    return TermCompiler_init(self, parent, searcher, boost);
}

TermCompiler*
TermCompiler_init(TermCompiler *self, Query *parent, Searcher *searcher,
                  float boost) {
    Schema     *schema  = Searcher_Get_Schema(searcher);
    TermQuery  *tparent = (TermQuery*)parent;
    Similarity *sim     = Schema_Fetch_Sim(schema, tparent->field);

    // Try harder to get a Similarity if necessary.
    if (!sim) { sim = Schema_Get_Similarity(schema); }

    // Init.
    Compiler_init((Compiler*)self, parent, searcher, sim, boost);
    self->normalized_weight = 0.0f;
    self->query_norm_factor = 0.0f;

    // Derive.
    int32_t doc_max  = Searcher_Doc_Max(searcher);
    int32_t doc_freq = Searcher_Doc_Freq(searcher, tparent->field,
                                         tparent->term);
    self->idf = Sim_IDF(sim, doc_freq, doc_max);

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
    self->raw_weight = self->idf * self->boost;

    return self;
}

bool_t
TermCompiler_equals(TermCompiler *self, Obj *other) {
    TermCompiler *twin = (TermCompiler*)other;
    if (!Compiler_equals((Compiler*)self, other))           { return false; }
    if (!Obj_Is_A(other, TERMCOMPILER))                     { return false; }
    if (self->idf != twin->idf)                             { return false; }
    if (self->raw_weight != twin->raw_weight)               { return false; }
    if (self->query_norm_factor != twin->query_norm_factor) { return false; }
    if (self->normalized_weight != twin->normalized_weight) { return false; }
    return true;
}

void
TermCompiler_serialize(TermCompiler *self, OutStream *outstream) {
    Compiler_serialize((Compiler*)self, outstream);
    OutStream_Write_F32(outstream, self->idf);
    OutStream_Write_F32(outstream, self->raw_weight);
    OutStream_Write_F32(outstream, self->query_norm_factor);
    OutStream_Write_F32(outstream, self->normalized_weight);
}

TermCompiler*
TermCompiler_deserialize(TermCompiler *self, InStream *instream) {
    self = self ? self : (TermCompiler*)VTable_Make_Obj(TERMCOMPILER);
    Compiler_deserialize((Compiler*)self, instream);
    self->idf               = InStream_Read_F32(instream);
    self->raw_weight        = InStream_Read_F32(instream);
    self->query_norm_factor = InStream_Read_F32(instream);
    self->normalized_weight = InStream_Read_F32(instream);
    return self;
}

float
TermCompiler_sum_of_squared_weights(TermCompiler *self) {
    return self->raw_weight * self->raw_weight;
}

void
TermCompiler_apply_norm_factor(TermCompiler *self, float query_norm_factor) {
    self->query_norm_factor = query_norm_factor;

    /* Multiply raw weight by the idf and norm_q factors in this:
     *
     *      (tf_q * idf_q / norm_q)
     *
     * Note: factoring in IDF a second time is correct.  See formula.
     */
    self->normalized_weight
        = self->raw_weight * self->idf * query_norm_factor;
}

float
TermCompiler_get_weight(TermCompiler *self) {
    return self->normalized_weight;
}

Matcher*
TermCompiler_make_matcher(TermCompiler *self, SegReader *reader,
                          bool_t need_score) {
    TermQuery *tparent = (TermQuery*)self->parent;
    PostingListReader *plist_reader
        = (PostingListReader*)SegReader_Fetch(
              reader, VTable_Get_Name(POSTINGLISTREADER));
    PostingList *plist = plist_reader
                         ? PListReader_Posting_List(plist_reader, tparent->field, tparent->term)
                         : NULL;

    if (plist == NULL || PList_Get_Doc_Freq(plist) == 0) {
        DECREF(plist);
        return NULL;
    }
    else {
        Matcher *retval = PList_Make_Matcher(plist, self->sim,
                                             (Compiler*)self, need_score);
        DECREF(plist);
        return retval;
    }
}

VArray*
TermCompiler_highlight_spans(TermCompiler *self, Searcher *searcher,
                             DocVector *doc_vec, const CharBuf *field) {
    TermQuery *const  parent = (TermQuery*)self->parent;
    VArray          *spans   = VA_new(0);
    TermVector *term_vector;
    I32Array *starts, *ends;
    uint32_t i, max;
    UNUSED_VAR(searcher);

    if (!CB_Equals(parent->field, (Obj*)field)) { return spans; }

    // Add all starts and ends.
    term_vector = DocVec_Term_Vector(doc_vec, field, (CharBuf*)parent->term);
    if (!term_vector) { return spans; }

    starts = TV_Get_Start_Offsets(term_vector);
    ends   = TV_Get_End_Offsets(term_vector);
    for (i = 0, max = I32Arr_Get_Size(starts); i < max; i++) {
        int32_t start  = I32Arr_Get(starts, i);
        int32_t length = I32Arr_Get(ends, i) - start;
        VA_Push(spans,
                (Obj*)Span_new(start, length, TermCompiler_Get_Weight(self)));
    }

    DECREF(term_vector);
    return spans;
}


