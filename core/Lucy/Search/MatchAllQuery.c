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

#define C_LUCY_MATCHALLQUERY
#define C_LUCY_MATCHALLCOMPILER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/MatchAllQuery.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Span.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Similarity.h"
#include "Lucy/Search/MatchAllMatcher.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

MatchAllQuery*
MatchAllQuery_new() {
    MatchAllQuery *self = (MatchAllQuery*)Class_Make_Obj(MATCHALLQUERY);
    return MatchAllQuery_init(self);
}

MatchAllQuery*
MatchAllQuery_init(MatchAllQuery *self) {
    return (MatchAllQuery*)Query_init((Query*)self, 0.0f);
}

bool
MatchAllQuery_Equals_IMP(MatchAllQuery *self, Obj *other) {
    if (!Obj_is_a(other, MATCHALLQUERY)) { return false; }
    MatchAllQueryIVARS *const ivars = MatchAllQuery_IVARS(self);
    MatchAllQueryIVARS *const ovars = MatchAllQuery_IVARS((MatchAllQuery*)other);
    if (ivars->boost != ovars->boost)    { return false; }
    return true;
}

String*
MatchAllQuery_To_String_IMP(MatchAllQuery *self) {
    UNUSED_VAR(self);
    return Str_new_from_trusted_utf8("[MATCHALL]", 10);
}

Compiler*
MatchAllQuery_Make_Compiler_IMP(MatchAllQuery *self, Searcher *searcher,
                                float boost) {
    UNUSED_VAR(self);
    UNUSED_VAR(searcher);
    return (Compiler*)MatchAllCompiler_new(boost);
}

/**********************************************************************/

MatchAllCompiler*
MatchAllCompiler_new(float boost) {
    MatchAllCompiler *self
        = (MatchAllCompiler*)Class_Make_Obj(MATCHALLCOMPILER);
    return MatchAllCompiler_init(self, boost);
}

MatchAllCompiler*
MatchAllCompiler_init(MatchAllCompiler *self, float boost) {
    MatchAllCompiler_IVARS(self)->boost = boost;
    return (MatchAllCompiler*)Compiler_init((Compiler*)self);
}

bool
MatchAllCompiler_Equals_IMP(MatchAllCompiler *self, Obj *other) {
    if ((MatchAllCompiler*)other == self)   { return true; }
    if (!Obj_is_a(other, MATCHALLCOMPILER)) { return false; }
    MatchAllCompilerIVARS *const ivars = MatchAllCompiler_IVARS(self);
    MatchAllCompilerIVARS *const ovars
        = MatchAllCompiler_IVARS((MatchAllCompiler*)other);
    if (ivars->boost != ovars->boost)       { return false; }
    return true;
}

void
MatchAllCompiler_Serialize_IMP(MatchAllCompiler *self, OutStream *outstream) {
    OutStream_Write_F32(outstream, MatchAllCompiler_IVARS(self)->boost);
}

MatchAllCompiler*
MatchAllCompiler_Deserialize_IMP(MatchAllCompiler *self, InStream *instream) {
    MatchAllCompiler_IVARS(self)->boost  = InStream_Read_F32(instream);
    return self;
}

Matcher*
MatchAllCompiler_Make_Matcher_IMP(MatchAllCompiler *self, SegReader *reader,
                                  bool need_score) {
    UNUSED_VAR(self);
    UNUSED_VAR(need_score);
    float weight = MatchAllCompiler_IVARS(self)->boost;
    return (Matcher*)MatchAllMatcher_new(weight, SegReader_Doc_Max(reader));
}

