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

#define C_LUCY_RANGEQUERY
#define C_LUCY_RANGECOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/RangeQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/RangeMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

// Determine the lowest ordinal that should match.
static int32_t
S_find_lower_bound(RangeCompiler *self, SortCache *sort_cache);

// Determine the highest ordinal that should match.
static int32_t
S_find_upper_bound(RangeCompiler *self, SortCache *sort_cache);

RangeQuery*
RangeQuery_new(String *field, Obj *lower_term, Obj *upper_term,
               bool include_lower, bool include_upper) {
    RangeQuery *self = (RangeQuery*)Class_Make_Obj(RANGEQUERY);
    return RangeQuery_init(self, field, lower_term, upper_term,
                           include_lower, include_upper);
}

RangeQuery*
RangeQuery_init(RangeQuery *self, String *field, Obj *lower_term,
                Obj *upper_term, bool include_lower, bool include_upper) {
    Query_init((Query*)self, 0.0f);
    RangeQueryIVARS *const ivars = RangeQuery_IVARS(self);
    ivars->field          = Str_Clone(field);
    ivars->lower_term     = lower_term ? Obj_Clone(lower_term) : NULL;
    ivars->upper_term     = upper_term ? Obj_Clone(upper_term) : NULL;
    ivars->include_lower  = include_lower;
    ivars->include_upper  = include_upper;
    if (!upper_term && !lower_term) {
        DECREF(self);
        self = NULL;
        THROW(ERR, "Must supply at least one of 'upper_term' and 'lower_term'");
    }
    return self;
}

void
RangeQuery_Destroy_IMP(RangeQuery *self) {
    RangeQueryIVARS *const ivars = RangeQuery_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->lower_term);
    DECREF(ivars->upper_term);
    SUPER_DESTROY(self, RANGEQUERY);
}

bool
RangeQuery_Equals_IMP(RangeQuery *self, Obj *other) {
    if ((RangeQuery*)other == self)                    { return true; }
    if (!Obj_Is_A(other, RANGEQUERY))                  { return false; }
    RangeQueryIVARS *const ivars = RangeQuery_IVARS(self);
    RangeQueryIVARS *const ovars = RangeQuery_IVARS((RangeQuery*)other);
    if (ivars->boost != ovars->boost)                  { return false; }
    if (!Str_Equals(ivars->field, (Obj*)ovars->field)) { return false; }
    if (ivars->lower_term && !ovars->lower_term)       { return false; }
    if (ivars->upper_term && !ovars->upper_term)       { return false; }
    if (!ivars->lower_term && ovars->lower_term)       { return false; }
    if (!ivars->upper_term && ovars->upper_term)       { return false; }
    if (ivars->lower_term
        && !Obj_Equals(ivars->lower_term, ovars->lower_term)) { return false; }
    if (ivars->upper_term
        && !Obj_Equals(ivars->upper_term, ovars->upper_term)) { return false; }
    if (ivars->include_lower != ovars->include_lower)         { return false; }
    if (ivars->include_upper != ovars->include_upper)         { return false; }
    return true;
}

String*
RangeQuery_To_String_IMP(RangeQuery *self) {
    RangeQueryIVARS *const ivars = RangeQuery_IVARS(self);
    String *lower_term_str = ivars->lower_term
                             ? Obj_To_String(ivars->lower_term)
                             : Str_new_from_trusted_utf8("*", 1);
    String *upper_term_str = ivars->upper_term
                             ? Obj_To_String(ivars->upper_term)
                             : Str_new_from_trusted_utf8("*", 1);
    String *retval = Str_newf("%o:%s%o TO %o%s", ivars->field,
                              ivars->include_lower ? "[" : "{",
                              lower_term_str,
                              upper_term_str,
                              ivars->include_upper ? "]" : "}"
                             );
    DECREF(upper_term_str);
    DECREF(lower_term_str);
    return retval;
}

void
RangeQuery_Serialize_IMP(RangeQuery *self, OutStream *outstream) {
    RangeQueryIVARS *const ivars = RangeQuery_IVARS(self);
    OutStream_Write_F32(outstream, ivars->boost);
    Freezer_serialize_string(ivars->field, outstream);
    if (ivars->lower_term) {
        OutStream_Write_U8(outstream, true);
        FREEZE(ivars->lower_term, outstream);
    }
    else {
        OutStream_Write_U8(outstream, false);
    }
    if (ivars->upper_term) {
        OutStream_Write_U8(outstream, true);
        FREEZE(ivars->upper_term, outstream);
    }
    else {
        OutStream_Write_U8(outstream, false);
    }
    OutStream_Write_U8(outstream, ivars->include_lower);
    OutStream_Write_U8(outstream, ivars->include_upper);
}

RangeQuery*
RangeQuery_Deserialize_IMP(RangeQuery *self, InStream *instream) {
    // Deserialize components.
    float boost = InStream_Read_F32(instream);
    String *field = Freezer_read_string(instream);
    Obj *lower_term = InStream_Read_U8(instream) ? THAW(instream) : NULL;
    Obj *upper_term = InStream_Read_U8(instream) ? THAW(instream) : NULL;
    bool include_lower = !!InStream_Read_U8(instream);
    bool include_upper = !!InStream_Read_U8(instream);

    // Init object.
    RangeQuery_init(self, field, lower_term, upper_term, include_lower,
                    include_upper);
    RangeQuery_Set_Boost(self, boost);

    DECREF(upper_term);
    DECREF(lower_term);
    DECREF(field);
    return self;
}

Obj*
RangeQuery_Dump_IMP(RangeQuery *self) {
    RangeQueryIVARS *ivars = RangeQuery_IVARS(self);
    RangeQuery_Dump_t super_dump
        = SUPER_METHOD_PTR(RANGEQUERY, LUCY_RangeQuery_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Utf8(dump, "field", 5, Freezer_dump((Obj*)ivars->field));
    if (ivars->lower_term) {
        Hash_Store_Utf8(dump, "lower_term", 10,
                        Freezer_dump((Obj*)ivars->lower_term));
    }
    if (ivars->upper_term) {
        Hash_Store_Utf8(dump, "upper_term", 10,
                        Freezer_dump((Obj*)ivars->upper_term));
    }
    Hash_Store_Utf8(dump, "include_lower", 13,
                    (Obj*)Bool_singleton(ivars->include_lower));
    Hash_Store_Utf8(dump, "include_upper", 13,
                    (Obj*)Bool_singleton(ivars->include_upper));
    return (Obj*)dump;
}

Obj*
RangeQuery_Load_IMP(RangeQuery *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    RangeQuery_Load_t super_load
        = SUPER_METHOD_PTR(RANGEQUERY, LUCY_RangeQuery_Load);
    RangeQuery *loaded = (RangeQuery*)super_load(self, dump);
    RangeQueryIVARS *loaded_ivars = RangeQuery_IVARS(loaded);
    Obj *field = CERTIFY(Hash_Fetch_Utf8(source, "field", 5), OBJ);
    loaded_ivars->field = (String*)CERTIFY(Freezer_load(field), STRING);
    Obj *lower_term = Hash_Fetch_Utf8(source, "lower_term", 10);
    if (lower_term) {
        loaded_ivars->lower_term
            = (Obj*)CERTIFY(Freezer_load(lower_term), OBJ);
    }
    Obj *upper_term = Hash_Fetch_Utf8(source, "upper_term", 10);
    if (upper_term) {
        loaded_ivars->upper_term
            = (Obj*)CERTIFY(Freezer_load(upper_term), OBJ);
    }
    Obj *include_lower
        = CERTIFY(Hash_Fetch_Utf8(source, "include_lower", 13), OBJ);
    loaded_ivars->include_lower = Obj_To_Bool(include_lower);
    Obj *include_upper
        = CERTIFY(Hash_Fetch_Utf8(source, "include_upper", 13), OBJ);
    loaded_ivars->include_upper = Obj_To_Bool(include_upper);
    return (Obj*)loaded;
}

RangeCompiler*
RangeQuery_Make_Compiler_IMP(RangeQuery *self, Searcher *searcher,
                             float boost, bool subordinate) {
    RangeCompiler *compiler = RangeCompiler_new(self, searcher, boost);
    if (!subordinate) {
        RangeCompiler_Normalize(compiler);
    }
    return compiler;
}

/**********************************************************************/

RangeCompiler*
RangeCompiler_new(RangeQuery *parent, Searcher *searcher, float boost) {
    RangeCompiler *self
        = (RangeCompiler*)Class_Make_Obj(RANGECOMPILER);
    return RangeCompiler_init(self, parent, searcher, boost);
}

RangeCompiler*
RangeCompiler_init(RangeCompiler *self, RangeQuery *parent,
                   Searcher *searcher, float boost) {
    return (RangeCompiler*)Compiler_init((Compiler*)self, (Query*)parent,
                                         searcher, NULL, boost);
}

Matcher*
RangeCompiler_Make_Matcher_IMP(RangeCompiler *self, SegReader *reader,
                               bool need_score) {
    RangeQuery *parent = (RangeQuery*)RangeCompiler_IVARS(self)->parent;
    String *field = RangeQuery_IVARS(parent)->field;
    SortReader *sort_reader
        = (SortReader*)SegReader_Fetch(reader, Class_Get_Name(SORTREADER));
    SortCache *sort_cache = sort_reader
                            ? SortReader_Fetch_Sort_Cache(sort_reader, field)
                            : NULL;
    UNUSED_VAR(need_score);

    if (!sort_cache) {
        return NULL;
    }
    else {
        int32_t lower = S_find_lower_bound(self, sort_cache);
        int32_t upper = S_find_upper_bound(self, sort_cache);
        int32_t max_ord = SortCache_Get_Cardinality(sort_cache) + 1;
        if (lower > max_ord || upper < 0) {
            return NULL;
        }
        else {
            int32_t doc_max = SegReader_Doc_Max(reader);
            return (Matcher*)RangeMatcher_new(lower, upper, sort_cache,
                                              doc_max);
        }
    }
}

static int32_t
S_find_lower_bound(RangeCompiler *self, SortCache *sort_cache) {
    RangeQuery *parent      = (RangeQuery*)RangeCompiler_IVARS(self)->parent;
    Obj        *lower_term  = RangeQuery_IVARS(parent)->lower_term;
    int32_t     lower_bound = 0;

    if (lower_term) {
        int32_t low_ord = SortCache_Find(sort_cache, lower_term);
        if (low_ord < 0) {
            // The supplied term is lower than all terms in the field.
            lower_bound = 0;
        }
        else {
            Obj *low_found = SortCache_Value(sort_cache, low_ord);
            bool exact_match = low_found == NULL
                                 ? false
                                 : Obj_Equals(lower_term, low_found);

            lower_bound = low_ord;
            if (!exact_match || !RangeQuery_IVARS(parent)->include_lower) {
                lower_bound++;
            }
            DECREF(low_found);
        }
    }

    return lower_bound;
}

static int32_t
S_find_upper_bound(RangeCompiler *self, SortCache *sort_cache) {
    RangeQuery *parent     = (RangeQuery*)RangeCompiler_IVARS(self)->parent;
    Obj        *upper_term = RangeQuery_IVARS(parent)->upper_term;
    int32_t     retval     = INT32_MAX;

    if (upper_term) {
        int32_t hi_ord = SortCache_Find(sort_cache, upper_term);
        if (hi_ord < 0) {
            // The supplied term is lower than all terms in the field.
            retval = -1;
        }
        else {
            Obj *hi_found = SortCache_Value(sort_cache, hi_ord);
            bool exact_match = hi_found == NULL
                                 ? false
                                 : Obj_Equals(upper_term, (Obj*)hi_found);

            retval = hi_ord;
            if (exact_match && !RangeQuery_IVARS(parent)->include_upper) {
                retval--;
            }
            DECREF(hi_found);
        }
    }

    return retval;
}


