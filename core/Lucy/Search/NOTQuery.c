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
    NOTQuery *self = (NOTQuery*)Class_Make_Obj(NOTQUERY);
    return NOTQuery_init(self, negated_query);
}

NOTQuery*
NOTQuery_init(NOTQuery *self, Query *negated_query) {
    self = (NOTQuery*)PolyQuery_init((PolyQuery*)self, NULL);
    NOTQuery_Set_Boost(self, 0.0f);
    NOTQuery_Add_Child(self, negated_query);
    return self;
}

Query*
NOTQuery_Get_Negated_Query_IMP(NOTQuery *self) {
    NOTQueryIVARS *const ivars = NOTQuery_IVARS(self);
    return (Query*)VA_Fetch(ivars->children, 0);
}

void
NOTQuery_Set_Negated_Query_IMP(NOTQuery *self, Query *negated_query) {
    NOTQueryIVARS *const ivars = NOTQuery_IVARS(self);
    VA_Store(ivars->children, 0, INCREF(negated_query));
}

String*
NOTQuery_To_String_IMP(NOTQuery *self) {
    NOTQueryIVARS *const ivars = NOTQuery_IVARS(self);
    String *neg_string = Obj_To_String(VA_Fetch(ivars->children, 0));
    String *retval = Str_newf("-%o", neg_string);
    DECREF(neg_string);
    return retval;
}

bool
NOTQuery_Equals_IMP(NOTQuery *self, Obj *other) {
    if ((NOTQuery*)other == self)   { return true; }
    if (!Obj_Is_A(other, NOTQUERY)) { return false; }
    NOTQuery_Equals_t super_equals
        = (NOTQuery_Equals_t)SUPER_METHOD_PTR(NOTQUERY, LUCY_NOTQuery_Equals);
    return super_equals(self, other);
}

Compiler*
NOTQuery_Make_Compiler_IMP(NOTQuery *self, Searcher *searcher, float boost,
                           bool subordinate) {
    NOTCompiler *compiler = NOTCompiler_new(self, searcher, boost);
    if (!subordinate) {
        NOTCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;
}

/**********************************************************************/

NOTCompiler*
NOTCompiler_new(NOTQuery *parent, Searcher *searcher, float boost) {
    NOTCompiler *self = (NOTCompiler*)Class_Make_Obj(NOTCOMPILER);
    return NOTCompiler_init(self, parent, searcher, boost);
}

NOTCompiler*
NOTCompiler_init(NOTCompiler *self, NOTQuery *parent, Searcher *searcher,
                 float boost) {
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher,
                      boost);
    return self;
}

float
NOTCompiler_Sum_Of_Squared_Weights_IMP(NOTCompiler *self) {
    UNUSED_VAR(self);
    return 0.0f;
}

VArray*
NOTCompiler_Highlight_Spans_IMP(NOTCompiler *self, Searcher *searcher,
                                DocVector *doc_vec, String *field) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    UNUSED_VAR(doc_vec);
    UNUSED_VAR(field);
    return VA_new(0);
}

Matcher*
NOTCompiler_Make_Matcher_IMP(NOTCompiler *self, SegReader *reader,
                             bool need_score) {
    NOTCompilerIVARS *const ivars = NOTCompiler_IVARS(self);
    Compiler *negated_compiler
        = (Compiler*)CERTIFY(VA_Fetch(ivars->children, 0), COMPILER);
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


