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

#define C_LUCY_PROXIMITYQUERY
#define C_LUCY_PROXIMITYCOMPILER

#include "Lucy/Util/ToolSet.h"

#include "LucyX/Search/ProximityQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/Posting.h"
#include "Lucy/Index/Posting/ScorePosting.h"
#include "Lucy/Index/PostingList.h"
#include "Lucy/Index/PostingListReader.h"
#include "Lucy/Index/SegPostingList.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/TermVector.h"
#include "Lucy/Plan/Schema.h"
#include "LucyX/Search/ProximityMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Search/TermQuery.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

// Shared initialization routine which assumes that it's ok to assume control
// over [field] and [terms], eating their refcounts.
static ProximityQuery*
S_do_init(ProximityQuery *self, CharBuf *field, VArray *terms, float boost,
          uint32_t within);

ProximityQuery*
ProximityQuery_new(const CharBuf *field, VArray *terms, uint32_t within) {
    ProximityQuery *self = (ProximityQuery*)VTable_Make_Obj(PROXIMITYQUERY);
    return ProximityQuery_init(self, field, terms, within);
}

ProximityQuery*
ProximityQuery_init(ProximityQuery *self, const CharBuf *field, VArray *terms,
                    uint32_t within) {
    return S_do_init(self, CB_Clone(field), VA_Clone(terms), 1.0f, within);
}

void
ProximityQuery_destroy(ProximityQuery *self) {
    DECREF(self->terms);
    DECREF(self->field);
    SUPER_DESTROY(self, PROXIMITYQUERY);
}

static ProximityQuery*
S_do_init(ProximityQuery *self, CharBuf *field, VArray *terms, float boost,
          uint32_t within) {
    uint32_t i, max;
    Query_init((Query*)self, boost);
    for (i = 0, max = VA_Get_Size(terms); i < max; i++) {
        CERTIFY(VA_Fetch(terms, i), OBJ);
    }
    self->field  = field;
    self->terms  = terms;
    self->within = within;
    return self;
}

void
ProximityQuery_serialize(ProximityQuery *self, OutStream *outstream) {
    OutStream_Write_F32(outstream, self->boost);
    CB_Serialize(self->field, outstream);
    VA_Serialize(self->terms, outstream);
    OutStream_Write_C32(outstream, self->within);
}

ProximityQuery*
ProximityQuery_deserialize(ProximityQuery *self, InStream *instream) {
    float     boost  = InStream_Read_F32(instream);
    CharBuf  *field  = CB_deserialize(NULL, instream);
    VArray   *terms  = VA_deserialize(NULL, instream);
    uint32_t  within = InStream_Read_C32(instream);
    self = self ? self : (ProximityQuery*)VTable_Make_Obj(PROXIMITYQUERY);
    return S_do_init(self, field, terms, boost, within);
}

bool_t
ProximityQuery_equals(ProximityQuery *self, Obj *other) {
    ProximityQuery *twin = (ProximityQuery*)other;
    if (twin == self)                     { return true; }
    if (!Obj_Is_A(other, PROXIMITYQUERY)) { return false; }
    if (self->boost != twin->boost)       { return false; }
    if (self->field && !twin->field)      { return false; }
    if (!self->field && twin->field)      { return false; }
    if (self->field && !CB_Equals(self->field, (Obj*)twin->field)) {
        return false;
    }
    if (!VA_Equals(twin->terms, (Obj*)self->terms)) { return false; }
    if (self->within != twin->within)               { return false; }
    return true;
}

CharBuf*
ProximityQuery_to_string(ProximityQuery *self) {
    uint32_t i;
    uint32_t num_terms = VA_Get_Size(self->terms);
    CharBuf *retval = CB_Clone(self->field);
    CB_Cat_Trusted_Str(retval, ":\"", 2);
    for (i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(self->terms, i);
        CharBuf *term_string = Obj_To_String(term);
        CB_Cat(retval, term_string);
        DECREF(term_string);
        if (i < num_terms - 1) {
            CB_Cat_Trusted_Str(retval, " ",  1);
        }
    }
    CB_Cat_Trusted_Str(retval, "\"", 1);
    CB_catf(retval, "~%u32", self->within);
    return retval;
}

Compiler*
ProximityQuery_make_compiler(ProximityQuery *self, Searcher *searcher,
                             float boost, bool_t subordinate) {
    if (VA_Get_Size(self->terms) == 1) {
        // Optimize for one-term "phrases".
        Obj *term = VA_Fetch(self->terms, 0);
        TermQuery *term_query = TermQuery_new(self->field, term);
        TermCompiler *term_compiler;
        TermQuery_Set_Boost(term_query, self->boost);
        term_compiler
            = (TermCompiler*)TermQuery_Make_Compiler(term_query, searcher,
                                                     boost, subordinate);
        DECREF(term_query);
        return (Compiler*)term_compiler;
    }
    else {
        ProximityCompiler *compiler
            = ProximityCompiler_new(self, searcher, boost, self->within);
        if (!subordinate) {
            ProximityCompiler_Normalize(compiler);
        }   
        return (Compiler*)compiler;
    }
}

CharBuf*
ProximityQuery_get_field(ProximityQuery *self) {
    return self->field;
}

VArray*
ProximityQuery_get_terms(ProximityQuery *self) {
    return self->terms;
}

uint32_t
ProximityQuery_get_within(ProximityQuery  *self) {
    return self->within;
}

/*********************************************************************/

ProximityCompiler*
ProximityCompiler_new(ProximityQuery *parent, Searcher *searcher, float boost,
                      uint32_t within) {
    ProximityCompiler *self =
        (ProximityCompiler*)VTable_Make_Obj(PROXIMITYCOMPILER);
    return ProximityCompiler_init(self, parent, searcher, boost, within);
}

ProximityCompiler*
ProximityCompiler_init(ProximityCompiler *self, ProximityQuery *parent,
                       Searcher *searcher, float boost, uint32_t within) {
    Schema     *schema = Searcher_Get_Schema(searcher);
    Similarity *sim    = Schema_Fetch_Sim(schema, parent->field);
    VArray     *terms  = parent->terms;
    uint32_t i, max;

    self->within = within;

    // Try harder to find a Similarity if necessary.
    if (!sim) { sim = Schema_Get_Similarity(schema); }

    // Init.
    Compiler_init((Compiler*)self, (Query*)parent, searcher, sim, boost);

    // Store IDF for the phrase.
    self->idf = 0;
    for (i = 0, max = VA_Get_Size(terms); i < max; i++) {
        Obj *term = VA_Fetch(terms, i);
        int32_t doc_max  = Searcher_Doc_Max(searcher);
        int32_t doc_freq = Searcher_Doc_Freq(searcher, parent->field, term);
        self->idf += Sim_IDF(sim, doc_freq, doc_max);
    }

    // Calculate raw weight.
    self->raw_weight = self->idf * self->boost;

    return self;
}

void
ProximityCompiler_serialize(ProximityCompiler *self, OutStream *outstream) {
    Compiler_serialize((Compiler*)self, outstream);
    OutStream_Write_F32(outstream, self->idf);
    OutStream_Write_F32(outstream, self->raw_weight);
    OutStream_Write_F32(outstream, self->query_norm_factor);
    OutStream_Write_F32(outstream, self->normalized_weight);
    OutStream_Write_C32(outstream, self->within);
}

ProximityCompiler*
ProximityCompiler_deserialize(ProximityCompiler *self, InStream *instream) {
    self = self ? self : (ProximityCompiler*)VTable_Make_Obj(PROXIMITYCOMPILER);
    Compiler_deserialize((Compiler*)self, instream);
    self->idf               = InStream_Read_F32(instream);
    self->raw_weight        = InStream_Read_F32(instream);
    self->query_norm_factor = InStream_Read_F32(instream);
    self->normalized_weight = InStream_Read_F32(instream);
    self->within            = InStream_Read_C32(instream);
    return self;
}

bool_t
ProximityCompiler_equals(ProximityCompiler *self, Obj *other) {
    ProximityCompiler *twin = (ProximityCompiler*)other;
    if (!Obj_Is_A(other, PROXIMITYCOMPILER))                { return false; }
    if (!Compiler_equals((Compiler*)self, other))           { return false; }
    if (self->idf != twin->idf)                             { return false; }
    if (self->raw_weight != twin->raw_weight)               { return false; }
    if (self->query_norm_factor != twin->query_norm_factor) { return false; }
    if (self->normalized_weight != twin->normalized_weight) { return false; }
    if (self->within            != twin->within)            { return false; }
    return true;
}

float
ProximityCompiler_get_weight(ProximityCompiler *self) {
    return self->normalized_weight;
}

float
ProximityCompiler_sum_of_squared_weights(ProximityCompiler *self) {
    return self->raw_weight * self->raw_weight;
}

void
ProximityCompiler_apply_norm_factor(ProximityCompiler *self, float factor) {
    self->query_norm_factor = factor;
    self->normalized_weight = self->raw_weight * self->idf * factor;
}

Matcher*
ProximityCompiler_make_matcher(ProximityCompiler *self, SegReader *reader,
                               bool_t need_score) {
    UNUSED_VAR(need_score);
    ProximityQuery *const parent = (ProximityQuery*)self->parent;
    VArray *const      terms     = parent->terms;
    uint32_t           num_terms = VA_Get_Size(terms);

    // Bail if there are no terms.
    if (!num_terms) { return NULL; }

    // Bail unless field is valid and posting type supports positions.
    Similarity *sim     = ProximityCompiler_Get_Similarity(self);
    Posting    *posting = Sim_Make_Posting(sim);
    if (posting == NULL || !Obj_Is_A((Obj*)posting, SCOREPOSTING)) {
        DECREF(posting);
        return NULL;
    }
    DECREF(posting);

    // Bail if there's no PostingListReader for this segment.
    PostingListReader *const plist_reader
        = (PostingListReader*)SegReader_Fetch(
              reader, VTable_Get_Name(POSTINGLISTREADER));
    if (!plist_reader) { return NULL; }

    // Look up each term.
    VArray  *plists = VA_new(num_terms);
    for (uint32_t i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(terms, i);
        PostingList *plist
            = PListReader_Posting_List(plist_reader, parent->field, term);

        // Bail if any one of the terms isn't in the index.
        if (!plist || !PList_Get_Doc_Freq(plist)) {
            DECREF(plist);
            DECREF(plists);
            return NULL;
        }
        VA_Push(plists, (Obj*)plist);
    }

    Matcher *retval
        = (Matcher*)ProximityMatcher_new(sim, plists, (Compiler*)self, self->within);
    DECREF(plists);
    return retval;
}

VArray*
ProximityCompiler_highlight_spans(ProximityCompiler *self, Searcher *searcher,
                                  DocVector *doc_vec, const CharBuf *field) {
    ProximityQuery *const parent = (ProximityQuery*)self->parent;
    VArray         *const terms  = parent->terms;
    VArray         *const spans  = VA_new(0);
    VArray         *term_vectors;
    BitVector      *posit_vec;
    BitVector      *other_posit_vec;
    uint32_t        i;
    const uint32_t  num_terms = VA_Get_Size(terms);
    uint32_t        num_tvs;
    UNUSED_VAR(searcher);

    // Bail if no terms or field doesn't match.
    if (!num_terms) { return spans; }
    if (!CB_Equals(field, (Obj*)parent->field)) { return spans; }

    term_vectors    = VA_new(num_terms);
    posit_vec       = BitVec_new(0);
    other_posit_vec = BitVec_new(0);
    for (i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(terms, i);
        TermVector *term_vector
            = DocVec_Term_Vector(doc_vec, field, (CharBuf*)term);

        // Bail if any term is missing.
        if (!term_vector) {
            break;
        }

        VA_Push(term_vectors, (Obj*)term_vector);

        if (i == 0) {
            // Set initial positions from first term.
            uint32_t j;
            I32Array *positions = TV_Get_Positions(term_vector);
            for (j = I32Arr_Get_Size(positions); j > 0; j--) {
                BitVec_Set(posit_vec, I32Arr_Get(positions, j - 1));
            }
        }
        else {
            // Filter positions using logical "and".
            uint32_t j;
            I32Array *positions = TV_Get_Positions(term_vector);

            BitVec_Clear_All(other_posit_vec);
            for (j = I32Arr_Get_Size(positions); j > 0; j--) {
                int32_t pos = I32Arr_Get(positions, j - 1) - i;
                if (pos >= 0) {
                    BitVec_Set(other_posit_vec, pos);
                }
            }
            BitVec_And(posit_vec, other_posit_vec);
        }
    }

    // Proceed only if all terms are present.
    num_tvs = VA_Get_Size(term_vectors);
    if (num_tvs == num_terms) {
        TermVector *first_tv = (TermVector*)VA_Fetch(term_vectors, 0);
        TermVector *last_tv
            = (TermVector*)VA_Fetch(term_vectors, num_tvs - 1);
        I32Array *tv_start_positions = TV_Get_Positions(first_tv);
        I32Array *tv_end_positions   = TV_Get_Positions(last_tv);
        I32Array *tv_start_offsets   = TV_Get_Start_Offsets(first_tv);
        I32Array *tv_end_offsets     = TV_Get_End_Offsets(last_tv);
        uint32_t  terms_max          = num_terms - 1;
        I32Array *valid_posits       = BitVec_To_Array(posit_vec);
        uint32_t  num_valid_posits   = I32Arr_Get_Size(valid_posits);
        uint32_t  j = 0;
        uint32_t  posit_tick;
        float     weight = ProximityCompiler_Get_Weight(self);
        i = 0;

        // Add only those starts/ends that belong to a valid position.
        for (posit_tick = 0; posit_tick < num_valid_posits; posit_tick++) {
            int32_t valid_start_posit = I32Arr_Get(valid_posits, posit_tick);
            int32_t valid_end_posit   = valid_start_posit + terms_max;
            int32_t start_offset = 0, end_offset = 0;
            uint32_t max;

            for (max = I32Arr_Get_Size(tv_start_positions); i < max; i++) {
                if (I32Arr_Get(tv_start_positions, i) == valid_start_posit) {
                    start_offset = I32Arr_Get(tv_start_offsets, i);
                    break;
                }
            }
            for (max = I32Arr_Get_Size(tv_end_positions); j < max; j++) {
                if (I32Arr_Get(tv_end_positions, j) == valid_end_posit) {
                    end_offset = I32Arr_Get(tv_end_offsets, j);
                    break;
                }
            }

            VA_Push(spans, (Obj*)Span_new(start_offset,
                                          end_offset - start_offset, weight));

            i++, j++;
        }

        DECREF(valid_posits);
    }

    DECREF(other_posit_vec);
    DECREF(posit_vec);
    DECREF(term_vectors);
    return spans;
}


