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

#define C_LUCY_NOMATCHQUERY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/NoMatchQuery.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/NoMatchMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

NoMatchQuery*
NoMatchQuery_new() {
    NoMatchQuery *self = (NoMatchQuery*)Class_Make_Obj(NOMATCHQUERY);
    return NoMatchQuery_init(self);
}

NoMatchQuery*
NoMatchQuery_init(NoMatchQuery *self) {
    NoMatchQueryIVARS *const ivars = NoMatchQuery_IVARS(self);
    Query_init((Query*)self, 0.0f);
    ivars->fails_to_match = true;
    return self;
}

bool
NoMatchQuery_Equals_IMP(NoMatchQuery *self, Obj *other) {
    if (!Obj_Is_A(other, NOMATCHQUERY))                     { return false; }
    NoMatchQueryIVARS *const ivars = NoMatchQuery_IVARS(self);
    NoMatchQueryIVARS *const ovars = NoMatchQuery_IVARS((NoMatchQuery*)other);
    if (ivars->boost != ovars->boost)                       { return false; }
    if (!!ivars->fails_to_match != !!ovars->fails_to_match) { return false; }
    return true;
}

String*
NoMatchQuery_To_String_IMP(NoMatchQuery *self) {
    UNUSED_VAR(self);
    return Str_new_from_trusted_utf8("[NOMATCH]", 9);
}

Compiler*
NoMatchQuery_Make_Compiler_IMP(NoMatchQuery *self, Searcher *searcher,
                               float boost, bool subordinate) {
    NoMatchCompiler *compiler = NoMatchCompiler_new(self, searcher, boost);
    if (!subordinate) {
        NoMatchCompiler_Normalize(compiler);
    }
    return (Compiler*)compiler;
}

void
NoMatchQuery_Set_Fails_To_Match_IMP(NoMatchQuery *self, bool fails_to_match) {
    NoMatchQuery_IVARS(self)->fails_to_match = fails_to_match;
}

bool
NoMatchQuery_Get_Fails_To_Match_IMP(NoMatchQuery *self) {
    return NoMatchQuery_IVARS(self)->fails_to_match;
}

Obj*
NoMatchQuery_Dump_IMP(NoMatchQuery *self) {
    NoMatchQueryIVARS *const ivars = NoMatchQuery_IVARS(self);
    NoMatchQuery_Dump_t super_dump
        = SUPER_METHOD_PTR(NOMATCHQUERY, LUCY_NoMatchQuery_Dump);
    Hash *dump = (Hash*)CERTIFY(super_dump(self), HASH);
    Hash_Store_Utf8(dump, "fails_to_match", 14,
                    (Obj*)Bool_singleton(ivars->fails_to_match));
    return (Obj*)dump;
}

NoMatchQuery*
NoMatchQuery_Load_IMP(NoMatchQuery *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    NoMatchQuery_Load_t super_load
        = SUPER_METHOD_PTR(NOMATCHQUERY, LUCY_NoMatchQuery_Load);
    NoMatchQuery *loaded = super_load(self, dump);
    Obj *fails = CFISH_Hash_Fetch_Utf8(source, "fails_to_match", 14);
    NoMatchQuery_IVARS(loaded)->fails_to_match
        = fails ? Obj_To_Bool(fails) : true;
    return loaded;
}

void
NoMatchQuery_Serialize_IMP(NoMatchQuery *self, OutStream *outstream) {
    NoMatchQueryIVARS *const ivars = NoMatchQuery_IVARS(self);
    OutStream_Write_I8(outstream, !!ivars->fails_to_match);
}

NoMatchQuery*
NoMatchQuery_Deserialize_IMP(NoMatchQuery *self, InStream *instream) {
    NoMatchQuery_init(self);
    NoMatchQuery_IVARS(self)->fails_to_match = !!InStream_Read_I8(instream);
    return self;
}

/**********************************************************************/

NoMatchCompiler*
NoMatchCompiler_new(NoMatchQuery *parent, Searcher *searcher,
                    float boost) {
    NoMatchCompiler *self
        = (NoMatchCompiler*)Class_Make_Obj(NOMATCHCOMPILER);
    return NoMatchCompiler_init(self, parent, searcher, boost);
}

NoMatchCompiler*
NoMatchCompiler_init(NoMatchCompiler *self, NoMatchQuery *parent,
                     Searcher *searcher, float boost) {
    return (NoMatchCompiler*)Compiler_init((Compiler*)self, (Query*)parent,
                                           searcher, NULL, boost);
}

Matcher*
NoMatchCompiler_Make_Matcher_IMP(NoMatchCompiler *self, SegReader *reader,
                                 bool need_score) {
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(need_score);
    return (Matcher*)NoMatchMatcher_new();
}


