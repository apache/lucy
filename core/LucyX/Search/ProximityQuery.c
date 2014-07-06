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

#include "Clownfish/CharBuf.h"
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
S_do_init(ProximityQuery *self, String *field, VArray *terms, float boost,
          uint32_t within);

ProximityQuery*
ProximityQuery_new(String *field, VArray *terms, uint32_t within) {
    ProximityQuery *self = (ProximityQuery*)Class_Make_Obj(PROXIMITYQUERY);
    return ProximityQuery_init(self, field, terms, within);
}

ProximityQuery*
ProximityQuery_init(ProximityQuery *self, String *field, VArray *terms,
                    uint32_t within) {
    return S_do_init(self, Str_Clone(field), VA_Clone(terms), 1.0f, within);
}

void
ProximityQuery_Destroy_IMP(ProximityQuery *self) {
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    DECREF(ivars->terms);
    DECREF(ivars->field);
    SUPER_DESTROY(self, PROXIMITYQUERY);
}

static ProximityQuery*
S_do_init(ProximityQuery *self, String *field, VArray *terms, float boost,
          uint32_t within) {
    Query_init((Query*)self, boost);
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(terms); i < max; i++) {
        CERTIFY(VA_Fetch(terms, i), OBJ);
    }
    ivars->field  = field;
    ivars->terms  = terms;
    ivars->within = within;
    return self;
}

void
ProximityQuery_Serialize_IMP(ProximityQuery *self, OutStream *outstream) {
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    OutStream_Write_F32(outstream, ivars->boost);
    Freezer_serialize_string(ivars->field, outstream);
    Freezer_serialize_varray(ivars->terms, outstream);
    OutStream_Write_C32(outstream, ivars->within);
}

ProximityQuery*
ProximityQuery_Deserialize_IMP(ProximityQuery *self, InStream *instream) {
    float boost = InStream_Read_F32(instream);
    String *field = Freezer_read_string(instream);
    VArray *terms = Freezer_read_varray(instream);
    uint32_t within = InStream_Read_C32(instream);
    return S_do_init(self, field, terms, boost, within);
}

Obj*
ProximityQuery_Dump_IMP(ProximityQuery *self) {
    ProximityQueryIVARS *ivars = ProximityQuery_IVARS(self);
    ProximityQuery_Dump_t super_dump
        = SUPER_METHOD_PTR(PROXIMITYQUERY, LUCY_ProximityQuery_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Utf8(dump, "field", 5, Freezer_dump((Obj*)ivars->field));
    Hash_Store_Utf8(dump, "terms", 5, Freezer_dump((Obj*)ivars->terms));
    Hash_Store_Utf8(dump, "within", 6,
                    (Obj*)Str_newf("%i64", (int64_t)ivars->within));
    return (Obj*)dump;
}

Obj*
ProximityQuery_Load_IMP(ProximityQuery *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    ProximityQuery_Load_t super_load
        = SUPER_METHOD_PTR(PROXIMITYQUERY, LUCY_ProximityQuery_Load);
    ProximityQuery *loaded = (ProximityQuery*)super_load(self, dump);
    ProximityQueryIVARS *loaded_ivars = ProximityQuery_IVARS(loaded);
    Obj *field = CERTIFY(Hash_Fetch_Utf8(source, "field", 5), OBJ);
    loaded_ivars->field = (String*)CERTIFY(Freezer_load(field), STRING);
    Obj *terms = CERTIFY(Hash_Fetch_Utf8(source, "terms", 5), OBJ);
    loaded_ivars->terms = (VArray*)CERTIFY(Freezer_load(terms), VARRAY);
    Obj *within = CERTIFY(Hash_Fetch_Utf8(source, "within", 6), OBJ);
    loaded_ivars->within = (uint32_t)Obj_To_I64(within);
    return (Obj*)loaded;
}

bool
ProximityQuery_Equals_IMP(ProximityQuery *self, Obj *other) {
    if ((ProximityQuery*)other == self)   { return true; }
    if (!Obj_Is_A(other, PROXIMITYQUERY)) { return false; }
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    ProximityQueryIVARS *const ovars
        = ProximityQuery_IVARS((ProximityQuery*)other);

    if (ivars->boost != ovars->boost)       { return false; }
    if (ivars->field && !ovars->field)      { return false; }
    if (!ivars->field && ovars->field)      { return false; }
    if (ivars->field && !Str_Equals(ivars->field, (Obj*)ovars->field)) {
        return false;
    }
    if (!VA_Equals(ovars->terms, (Obj*)ivars->terms)) { return false; }
    if (ivars->within != ovars->within)               { return false; }
    return true;
}

String*
ProximityQuery_To_String_IMP(ProximityQuery *self) {
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    uint32_t num_terms = VA_Get_Size(ivars->terms);
    CharBuf *buf = CB_new_from_str(ivars->field);
    CB_Cat_Trusted_Utf8(buf, ":\"", 2);
    for (uint32_t i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(ivars->terms, i);
        String *term_string = Obj_To_String(term);
        CB_Cat(buf, term_string);
        DECREF(term_string);
        if (i < num_terms - 1) {
            CB_Cat_Trusted_Utf8(buf, " ",  1);
        }
    }
    CB_Cat_Trusted_Utf8(buf, "\"", 1);
    CB_catf(buf, "~%u32", ivars->within);
    String *retval = CB_Yield_String(buf);
    DECREF(buf);
    return retval;
}

Compiler*
ProximityQuery_Make_Compiler_IMP(ProximityQuery *self, Searcher *searcher,
                                 float boost, bool subordinate) {
    ProximityQueryIVARS *const ivars = ProximityQuery_IVARS(self);
    if (VA_Get_Size(ivars->terms) == 1) {
        // Optimize for one-term "phrases".
        Obj *term = VA_Fetch(ivars->terms, 0);
        TermQuery *term_query = TermQuery_new(ivars->field, term);
        TermQuery_Set_Boost(term_query, ivars->boost);
        TermCompiler *term_compiler
            = (TermCompiler*)TermQuery_Make_Compiler(term_query, searcher,
                                                     boost, subordinate);
        DECREF(term_query);
        return (Compiler*)term_compiler;
    }
    else {
        ProximityCompiler *compiler
            = ProximityCompiler_new(self, searcher, boost, ivars->within);
        if (!subordinate) {
            ProximityCompiler_Normalize(compiler);
        }
        return (Compiler*)compiler;
    }
}

String*
ProximityQuery_Get_Field_IMP(ProximityQuery *self) {
    return ProximityQuery_IVARS(self)->field;
}

VArray*
ProximityQuery_Get_Terms_IMP(ProximityQuery *self) {
    return ProximityQuery_IVARS(self)->terms;
}

uint32_t
ProximityQuery_Get_Within_IMP(ProximityQuery  *self) {
    return ProximityQuery_IVARS(self)->within;
}

/*********************************************************************/

ProximityCompiler*
ProximityCompiler_new(ProximityQuery *parent, Searcher *searcher, float boost,
                      uint32_t within) {
    ProximityCompiler *self =
        (ProximityCompiler*)Class_Make_Obj(PROXIMITYCOMPILER);
    return ProximityCompiler_init(self, parent, searcher, boost, within);
}

ProximityCompiler*
ProximityCompiler_init(ProximityCompiler *self, ProximityQuery *parent,
                       Searcher *searcher, float boost, uint32_t within) {
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    ProximityQueryIVARS *const parent_ivars = ProximityQuery_IVARS(parent);
    Schema     *schema = Searcher_Get_Schema(searcher);
    Similarity *sim    = Schema_Fetch_Sim(schema, parent_ivars->field);
    VArray     *terms  = parent_ivars->terms;

    ivars->within = within;

    // Try harder to find a Similarity if necessary.
    if (!sim) { sim = Schema_Get_Similarity(schema); }

    // Init.
    Compiler_init((Compiler*)self, (Query*)parent, searcher, sim, boost);

    // Store IDF for the phrase.
    ivars->idf = 0;
    for (uint32_t i = 0, max = VA_Get_Size(terms); i < max; i++) {
        Obj *term = VA_Fetch(terms, i);
        int32_t doc_max  = Searcher_Doc_Max(searcher);
        int32_t doc_freq
            = Searcher_Doc_Freq(searcher, parent_ivars->field,term);
        ivars->idf += Sim_IDF(sim, doc_freq, doc_max);
    }

    // Calculate raw weight.
    ivars->raw_weight = ivars->idf * ivars->boost;

    return self;
}

void
ProximityCompiler_Serialize_IMP(ProximityCompiler *self,
                                OutStream *outstream) {
    ProximityCompiler_Serialize_t super_serialize
            = SUPER_METHOD_PTR(PROXIMITYCOMPILER, LUCY_ProximityCompiler_Serialize);
    super_serialize(self, outstream);
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    OutStream_Write_F32(outstream, ivars->idf);
    OutStream_Write_F32(outstream, ivars->raw_weight);
    OutStream_Write_F32(outstream, ivars->query_norm_factor);
    OutStream_Write_F32(outstream, ivars->normalized_weight);
    OutStream_Write_C32(outstream, ivars->within);
}

ProximityCompiler*
ProximityCompiler_Deserialize_IMP(ProximityCompiler *self,
                                  InStream *instream) {
    ProximityCompiler_Deserialize_t super_deserialize
            = SUPER_METHOD_PTR(PROXIMITYCOMPILER, LUCY_ProximityCompiler_Deserialize);
    self = super_deserialize(self, instream);
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    ivars->idf               = InStream_Read_F32(instream);
    ivars->raw_weight        = InStream_Read_F32(instream);
    ivars->query_norm_factor = InStream_Read_F32(instream);
    ivars->normalized_weight = InStream_Read_F32(instream);
    ivars->within            = InStream_Read_C32(instream);
    return self;
}

bool
ProximityCompiler_Equals_IMP(ProximityCompiler *self, Obj *other) {
    if ((ProximityCompiler*)other == self)        { return true; }
    if (!Obj_Is_A(other, PROXIMITYCOMPILER))      { return false; }
    ProximityCompiler_Equals_t super_equals
        = (ProximityCompiler_Equals_t)SUPER_METHOD_PTR(PROXIMITYCOMPILER,
                                                       LUCY_ProximityCompiler_Equals);
    if (!super_equals(self, other)) { return false; }
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    ProximityCompilerIVARS *const ovars
        = ProximityCompiler_IVARS((ProximityCompiler*)other);
    if (ivars->idf != ovars->idf)                             { return false; }
    if (ivars->raw_weight != ovars->raw_weight)               { return false; }
    if (ivars->query_norm_factor != ovars->query_norm_factor) { return false; }
    if (ivars->normalized_weight != ovars->normalized_weight) { return false; }
    if (ivars->within            != ovars->within)            { return false; }
    return true;
}

float
ProximityCompiler_Get_Weight_IMP(ProximityCompiler *self) {
    return ProximityCompiler_IVARS(self)->normalized_weight;
}

float
ProximityCompiler_Sum_Of_Squared_Weights_IMP(ProximityCompiler *self) {
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    return ivars->raw_weight * ivars->raw_weight;
}

void
ProximityCompiler_Apply_Norm_Factor_IMP(ProximityCompiler *self,
                                        float factor) {
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    ivars->query_norm_factor = factor;
    ivars->normalized_weight = ivars->raw_weight * ivars->idf * factor;
}

Matcher*
ProximityCompiler_Make_Matcher_IMP(ProximityCompiler *self, SegReader *reader,
                                   bool need_score) {
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    UNUSED_VAR(need_score);
    ProximityQueryIVARS *const parent_ivars
        = ProximityQuery_IVARS((ProximityQuery*)ivars->parent);
    VArray *const      terms     = parent_ivars->terms;
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
              reader, Class_Get_Name(POSTINGLISTREADER));
    if (!plist_reader) { return NULL; }

    // Look up each term.
    VArray  *plists = VA_new(num_terms);
    for (uint32_t i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(terms, i);
        PostingList *plist
            = PListReader_Posting_List(plist_reader, parent_ivars->field, term);

        // Bail if any one of the terms isn't in the index.
        if (!plist || !PList_Get_Doc_Freq(plist)) {
            DECREF(plist);
            DECREF(plists);
            return NULL;
        }
        VA_Push(plists, (Obj*)plist);
    }

    Matcher *retval
        = (Matcher*)ProximityMatcher_new(sim, plists, (Compiler*)self, ivars->within);
    DECREF(plists);
    return retval;
}

VArray*
ProximityCompiler_Highlight_Spans_IMP(ProximityCompiler *self,
                                      Searcher *searcher, DocVector *doc_vec,
                                      String *field) {
    ProximityCompilerIVARS *const ivars = ProximityCompiler_IVARS(self);
    ProximityQueryIVARS *const parent_ivars
        = ProximityQuery_IVARS((ProximityQuery*)ivars->parent);
    VArray         *const terms  = parent_ivars->terms;
    VArray         *const spans  = VA_new(0);
    const uint32_t  num_terms    = VA_Get_Size(terms);
    UNUSED_VAR(searcher);

    // Bail if no terms or field doesn't match.
    if (!num_terms) { return spans; }
    if (!Str_Equals(field, (Obj*)parent_ivars->field)) { return spans; }

    VArray      *term_vectors    = VA_new(num_terms);
    BitVector   *posit_vec       = BitVec_new(0);
    BitVector   *other_posit_vec = BitVec_new(0);
    for (uint32_t i = 0; i < num_terms; i++) {
        Obj *term = VA_Fetch(terms, i);
        TermVector *term_vector
            = DocVec_Term_Vector(doc_vec, field, (String*)term);

        // Bail if any term is missing.
        if (!term_vector) {
            break;
        }

        VA_Push(term_vectors, (Obj*)term_vector);

        if (i == 0) {
            // Set initial positions from first term.
            I32Array *positions = TV_Get_Positions(term_vector);
            for (uint32_t j = I32Arr_Get_Size(positions); j > 0; j--) {
                BitVec_Set(posit_vec, I32Arr_Get(positions, j - 1));
            }
        }
        else {
            // Filter positions using logical "and".
            I32Array *positions = TV_Get_Positions(term_vector);

            BitVec_Clear_All(other_posit_vec);
            for (uint32_t j = I32Arr_Get_Size(positions); j > 0; j--) {
                int32_t pos = I32Arr_Get(positions, j - 1) - i;
                if (pos >= 0) {
                    BitVec_Set(other_posit_vec, pos);
                }
            }
            BitVec_And(posit_vec, other_posit_vec);
        }
    }

    // Proceed only if all terms are present.
    uint32_t num_tvs = VA_Get_Size(term_vectors);
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
        float     weight = ProximityCompiler_Get_Weight(self);
        uint32_t  i = 0;

        // Add only those starts/ends that belong to a valid position.
        for (uint32_t posit_tick = 0; posit_tick < num_valid_posits; posit_tick++) {
            int32_t valid_start_posit = I32Arr_Get(valid_posits, posit_tick);
            int32_t valid_end_posit   = valid_start_posit + terms_max;
            int32_t start_offset = 0, end_offset = 0;

            for (uint32_t max = I32Arr_Get_Size(tv_start_positions); i < max; i++) {
                if (I32Arr_Get(tv_start_positions, i) == valid_start_posit) {
                    start_offset = I32Arr_Get(tv_start_offsets, i);
                    break;
                }
            }
            for (uint32_t max = I32Arr_Get_Size(tv_end_positions); j < max; j++) {
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


