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

#define C_LUCY_ANDQUERY
#define C_LUCY_ANDCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/ANDMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

ANDQuery*
ANDQuery_new(VArray *children) {
    ANDQuery *self = (ANDQuery*)VTable_Make_Obj(ANDQUERY);
    return ANDQuery_init(self, children);
}

ANDQuery*
ANDQuery_init(ANDQuery *self, VArray *children) {
    return (ANDQuery*)PolyQuery_init((PolyQuery*)self, children);
}

CharBuf*
ANDQuery_to_string(ANDQuery *self) {
    uint32_t num_kids = VA_Get_Size(self->children);
    if (!num_kids) { return CB_new_from_trusted_utf8("()", 2); }
    else {
        CharBuf *retval = CB_new_from_trusted_utf8("(", 1);
        uint32_t i;
        for (i = 0; i < num_kids; i++) {
            CharBuf *kid_string = Obj_To_String(VA_Fetch(self->children, i));
            CB_Cat(retval, kid_string);
            DECREF(kid_string);
            if (i == num_kids - 1) {
                CB_Cat_Trusted_Str(retval, ")", 1);
            }
            else {
                CB_Cat_Trusted_Str(retval, " AND ", 5);
            }
        }
        return retval;
    }
}


bool_t
ANDQuery_equals(ANDQuery *self, Obj *other) {
    if ((ANDQuery*)other == self)   { return true; }
    if (!Obj_Is_A(other, ANDQUERY)) { return false; }
    return PolyQuery_equals((PolyQuery*)self, other);
}

Compiler*
ANDQuery_make_compiler(ANDQuery *self, Searcher *searcher, float boost,
                       bool_t subordinate) {
    ANDCompiler *compiler = ANDCompiler_new(self, searcher, boost);
    if (!subordinate) {
        ANDCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;
}

/**********************************************************************/

ANDCompiler*
ANDCompiler_new(ANDQuery *parent, Searcher *searcher, float boost) {
    ANDCompiler *self = (ANDCompiler*)VTable_Make_Obj(ANDCOMPILER);
    return ANDCompiler_init(self, parent, searcher, boost);
}

ANDCompiler*
ANDCompiler_init(ANDCompiler *self, ANDQuery *parent, Searcher *searcher,
                 float boost) {
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher,
                      boost);
    return self;
}

Matcher*
ANDCompiler_make_matcher(ANDCompiler *self, SegReader *reader,
                         bool_t need_score) {
    uint32_t num_kids = VA_Get_Size(self->children);

    if (num_kids == 1) {
        Compiler *only_child = (Compiler*)VA_Fetch(self->children, 0);
        return Compiler_Make_Matcher(only_child, reader, need_score);
    }
    else {
        uint32_t i;
        VArray *child_matchers = VA_new(num_kids);

        // Add child matchers one by one.
        for (i = 0; i < num_kids; i++) {
            Compiler *child = (Compiler*)VA_Fetch(self->children, i);
            Matcher *child_matcher
                = Compiler_Make_Matcher(child, reader, need_score);

            // If any required clause fails, the whole thing fails.
            if (child_matcher == NULL) {
                DECREF(child_matchers);
                return NULL;
            }
            else {
                VA_Push(child_matchers, (Obj*)child_matcher);
            }
        }

        Matcher *retval
            = (Matcher*)ANDMatcher_new(child_matchers,
                                       ANDCompiler_Get_Similarity(self));
        DECREF(child_matchers);
        return retval;

    }
}


