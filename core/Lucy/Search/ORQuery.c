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

#define C_LUCY_ORQUERY
#define C_LUCY_ORCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/ORQuery.h"

#include "Clownfish/CharBuf.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Search/ORMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

ORQuery*
ORQuery_new(VArray *children) {
    ORQuery *self = (ORQuery*)Class_Make_Obj(ORQUERY);
    return ORQuery_init(self, children);
}

ORQuery*
ORQuery_init(ORQuery *self, VArray *children) {
    return (ORQuery*)PolyQuery_init((PolyQuery*)self, children);
}

Compiler*
ORQuery_Make_Compiler_IMP(ORQuery *self, Searcher *searcher, float boost,
                          bool subordinate) {
    ORCompiler *compiler = ORCompiler_new(self, searcher, boost);
    if (!subordinate) {
        ORCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;
}

bool
ORQuery_Equals_IMP(ORQuery *self, Obj *other) {
    if ((ORQuery*)other == self)   { return true;  }
    if (!Obj_Is_A(other, ORQUERY)) { return false; }
    ORQuery_Equals_t super_equals
        = (ORQuery_Equals_t)SUPER_METHOD_PTR(ORQUERY, LUCY_ORQuery_Equals);
    return super_equals(self, other);
}

String*
ORQuery_To_String_IMP(ORQuery *self) {
    ORQueryIVARS *const ivars = ORQuery_IVARS(self);
    uint32_t num_kids = VA_Get_Size(ivars->children);
    if (!num_kids) { return Str_new_from_trusted_utf8("()", 2); }
    else {
        CharBuf *buf = CB_new_from_trusted_utf8("(", 1);
        uint32_t last_kid = num_kids - 1;
        for (uint32_t i = 0; i < num_kids; i++) {
            String *kid_string = Obj_To_String(VA_Fetch(ivars->children, i));
            CB_Cat(buf, kid_string);
            DECREF(kid_string);
            if (i == last_kid) {
                CB_Cat_Trusted_Utf8(buf, ")", 1);
            }
            else {
                CB_Cat_Trusted_Utf8(buf, " OR ", 4);
            }
        }
        String *retval = CB_Yield_String(buf);
        DECREF(buf);
        return retval;
    }
}

/**********************************************************************/

ORCompiler*
ORCompiler_new(ORQuery *parent, Searcher *searcher, float boost) {
    ORCompiler *self = (ORCompiler*)Class_Make_Obj(ORCOMPILER);
    return ORCompiler_init(self, parent, searcher, boost);
}

ORCompiler*
ORCompiler_init(ORCompiler *self, ORQuery *parent, Searcher *searcher,
                float boost) {
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher,
                      boost);
    return self;
}

Matcher*
ORCompiler_Make_Matcher_IMP(ORCompiler *self, SegReader *reader,
                            bool need_score) {
    ORCompilerIVARS *const ivars = ORCompiler_IVARS(self);
    uint32_t num_kids = VA_Get_Size(ivars->children);

    if (num_kids == 1) {
        // No need for an ORMatcher wrapper.
        Compiler *only_child = (Compiler*)VA_Fetch(ivars->children, 0);
        return Compiler_Make_Matcher(only_child, reader, need_score);
    }
    else {
        VArray *submatchers = VA_new(num_kids);
        uint32_t num_submatchers = 0;

        // Accumulate sub-matchers.
        for (uint32_t i = 0; i < num_kids; i++) {
            Compiler *child = (Compiler*)VA_Fetch(ivars->children, i);
            Matcher *submatcher
                = Compiler_Make_Matcher(child, reader, need_score);
            VA_Push(submatchers, (Obj*)submatcher);
            if (submatcher != NULL) {
                num_submatchers++;
            }
        }

        if (num_submatchers == 0) {
            // No possible matches, so return null.
            DECREF(submatchers);
            return NULL;
        }
        else {
            Similarity *sim    = ORCompiler_Get_Similarity(self);
            Matcher    *retval = need_score
                                 ? (Matcher*)ORScorer_new(submatchers, sim)
                                 : (Matcher*)ORMatcher_new(submatchers);
            DECREF(submatchers);
            return retval;
        }
    }
}


