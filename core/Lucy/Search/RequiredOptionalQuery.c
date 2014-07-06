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

#define C_LUCY_REQUIREDOPTIONALQUERY
#define C_LUCY_REQUIREDOPTIONALCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/RequiredOptionalQuery.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/RequiredOptionalMatcher.h"
#include "Lucy/Search/Searcher.h"

RequiredOptionalQuery*
ReqOptQuery_new(Query *required_query, Query *optional_query) {
    RequiredOptionalQuery *self
        = (RequiredOptionalQuery*)Class_Make_Obj(REQUIREDOPTIONALQUERY);
    return ReqOptQuery_init(self, required_query, optional_query);
}

RequiredOptionalQuery*
ReqOptQuery_init(RequiredOptionalQuery *self, Query *required_query,
                 Query *optional_query) {
    PolyQuery_init((PolyQuery*)self, NULL);
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    VA_Push(ivars->children, INCREF(required_query));
    VA_Push(ivars->children, INCREF(optional_query));
    return self;
}

Query*
ReqOptQuery_Get_Required_Query_IMP(RequiredOptionalQuery *self) {
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    return (Query*)VA_Fetch(ivars->children, 0);
}

void
ReqOptQuery_Set_Required_Query_IMP(RequiredOptionalQuery *self,
                                   Query *required_query) {
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    VA_Store(ivars->children, 0, INCREF(required_query));
}

Query*
ReqOptQuery_Get_Optional_Query_IMP(RequiredOptionalQuery *self) {
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    return (Query*)VA_Fetch(ivars->children, 1);
}

void
ReqOptQuery_Set_Optional_Query_IMP(RequiredOptionalQuery *self,
                                   Query *optional_query) {
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    VA_Store(ivars->children, 1, INCREF(optional_query));
}

String*
ReqOptQuery_To_String_IMP(RequiredOptionalQuery *self) {
    RequiredOptionalQueryIVARS *const ivars = ReqOptQuery_IVARS(self);
    String *req_string = Obj_To_String(VA_Fetch(ivars->children, 0));
    String *opt_string = Obj_To_String(VA_Fetch(ivars->children, 1));
    String *retval = Str_newf("(+%o %o)", req_string, opt_string);
    DECREF(opt_string);
    DECREF(req_string);
    return retval;
}

bool
ReqOptQuery_Equals_IMP(RequiredOptionalQuery *self, Obj *other) {
    if ((RequiredOptionalQuery*)other == self)   { return true;  }
    if (!Obj_Is_A(other, REQUIREDOPTIONALQUERY)) { return false; }
    ReqOptQuery_Equals_t super_equals
        = (ReqOptQuery_Equals_t)SUPER_METHOD_PTR(REQUIREDOPTIONALQUERY,
                                                 LUCY_ReqOptQuery_Equals);
    return super_equals(self, other);
}

Compiler*
ReqOptQuery_Make_Compiler_IMP(RequiredOptionalQuery *self, Searcher *searcher,
                              float boost, bool subordinate) {
    RequiredOptionalCompiler *compiler
        = ReqOptCompiler_new(self, searcher, boost);
    if (!subordinate) {
        ReqOptCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;
}

/**********************************************************************/

RequiredOptionalCompiler*
ReqOptCompiler_new(RequiredOptionalQuery *parent, Searcher *searcher,
                   float boost) {
    RequiredOptionalCompiler *self
        = (RequiredOptionalCompiler*)Class_Make_Obj(
              REQUIREDOPTIONALCOMPILER);
    return ReqOptCompiler_init(self, parent, searcher, boost);
}

RequiredOptionalCompiler*
ReqOptCompiler_init(RequiredOptionalCompiler *self,
                    RequiredOptionalQuery *parent,
                    Searcher *searcher, float boost) {
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher,
                      boost);
    return self;
}

Matcher*
ReqOptCompiler_Make_Matcher_IMP(RequiredOptionalCompiler *self,
                                SegReader *reader, bool need_score) {
    RequiredOptionalCompilerIVARS *const ivars = ReqOptCompiler_IVARS(self);
    Schema     *schema       = SegReader_Get_Schema(reader);
    Similarity *sim          = Schema_Get_Similarity(schema);
    Compiler   *req_compiler = (Compiler*)VA_Fetch(ivars->children, 0);
    Compiler   *opt_compiler = (Compiler*)VA_Fetch(ivars->children, 1);
    Matcher *req_matcher
        = Compiler_Make_Matcher(req_compiler, reader, need_score);
    Matcher *opt_matcher
        = Compiler_Make_Matcher(opt_compiler, reader, need_score);

    if (req_matcher == NULL) {
        // No required matcher, ergo no matches possible.
        DECREF(opt_matcher);
        return NULL;
    }
    else {
        Matcher *retval
            = (Matcher*)ReqOptMatcher_new(sim, req_matcher, opt_matcher);
        DECREF(opt_matcher);
        DECREF(req_matcher);
        return retval;
    }
}


