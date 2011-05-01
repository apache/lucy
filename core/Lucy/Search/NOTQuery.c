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

#define C_LUCY_NOTQUERY
#define C_LUCY_NOTCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Search/MatchAllMatcher.h"
#include "Lucy/Search/NOTMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

NOTQuery*
NOTQuery_new(Query *negated_query) {
    NOTQuery *self = (NOTQuery*)VTable_Make_Obj(NOTQUERY);
    return NOTQuery_init(self, negated_query);
}

NOTQuery*
NOTQuery_init(NOTQuery *self, Query *negated_query) {
    self = (NOTQuery*)PolyQuery_init((PolyQuery*)self, NULL);
    NOTQuery_Set_Boost(self, 0.0f);
    PolyQuery_add_child((PolyQuery*)self, negated_query);
    return self;
}

Query*
NOTQuery_get_negated_query(NOTQuery *self) {
    return (Query*)VA_Fetch(self->children, 0);
}

void
NOTQuery_set_negated_query(NOTQuery *self, Query *negated_query) {
    VA_Store(self->children, 0, INCREF(negated_query));
}

CharBuf*
NOTQuery_to_string(NOTQuery *self) {
    CharBuf *neg_string = Obj_To_String(VA_Fetch(self->children, 0));
    CharBuf *retval = CB_newf("-%o", neg_string);
    DECREF(neg_string);
    return retval;
}

bool_t
NOTQuery_equals(NOTQuery *self, Obj *other) {
    if ((NOTQuery*)other == self)   { return true; }
    if (!Obj_Is_A(other, NOTQUERY)) { return false; }
    return PolyQuery_equals((PolyQuery*)self, other);
}

Compiler*
NOTQuery_make_compiler(NOTQuery *self, Searcher *searcher, float boost) {
    return (Compiler*)NOTCompiler_new(self, searcher, boost);
}

/**********************************************************************/

NOTCompiler*
NOTCompiler_new(NOTQuery *parent, Searcher *searcher, float boost) {
    NOTCompiler *self = (NOTCompiler*)VTable_Make_Obj(NOTCOMPILER);
    return NOTCompiler_init(self, parent, searcher, boost);
}

NOTCompiler*
NOTCompiler_init(NOTCompiler *self, NOTQuery *parent, Searcher *searcher,
                 float boost) {
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher,
                      boost);
    NOTCompiler_Normalize(self);
    return self;
}

float
NOTCompiler_sum_of_squared_weights(NOTCompiler *self) {
    UNUSED_VAR(self);
    return 0.0f;
}

VArray*
NOTCompiler_highlight_spans(NOTCompiler *self, Searcher *searcher,
                            DocVector *doc_vec, const CharBuf *field) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(doc_vec);
    UNUSED_VAR(field);
    return VA_new(0);
}

Matcher*
NOTCompiler_make_matcher(NOTCompiler *self, SegReader *reader,
                         bool_t need_score) {
    Compiler *negated_compiler
        = (Compiler*)CERTIFY(VA_Fetch(self->children, 0), COMPILER);
    Matcher *negated_matcher
        = Compiler_Make_Matcher(negated_compiler, reader, false);
    UNUSED_VAR(need_score);

    if (negated_matcher == NULL) {
        float weight = NOTCompiler_Get_Weight(self);
        int32_t doc_max = SegReader_Doc_Max(reader);
        return (Matcher*)MatchAllMatcher_new(weight, doc_max);
    }
    else if (Obj_Is_A((Obj*)negated_matcher, MATCHALLMATCHER)) {
        DECREF(negated_matcher);
        return NULL;
    }
    else {
        int32_t doc_max = SegReader_Doc_Max(reader);
        Matcher *retval = (Matcher*)NOTMatcher_new(negated_matcher, doc_max);
        DECREF(negated_matcher);
        return retval;
    }
}


