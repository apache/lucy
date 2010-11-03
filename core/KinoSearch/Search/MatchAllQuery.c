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
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/MatchAllQuery.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/Span.h"
#include "KinoSearch/Index/DocVector.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Search/MatchAllScorer.h"
#include "KinoSearch/Search/Searcher.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/Freezer.h"

MatchAllQuery*
MatchAllQuery_new()
{
    MatchAllQuery *self = (MatchAllQuery*)VTable_Make_Obj(MATCHALLQUERY);
    return MatchAllQuery_init(self);
}

MatchAllQuery*
MatchAllQuery_init(MatchAllQuery *self)
{
    return (MatchAllQuery*)Query_init((Query*)self, 0.0f);
}

bool_t
MatchAllQuery_equals(MatchAllQuery *self, Obj *other)
{
    MatchAllQuery *evil_twin = (MatchAllQuery*)other;
    if (!Obj_Is_A(other, MATCHALLQUERY)) return false;
    if (self->boost != evil_twin->boost) return false;
    return true;
}

CharBuf*
MatchAllQuery_to_string(MatchAllQuery *self)
{
    UNUSED_VAR(self);
    return CB_new_from_trusted_utf8("[MATCHALL]", 10);
}

Compiler*
MatchAllQuery_make_compiler(MatchAllQuery *self, Searcher *searcher, 
                            float boost)
{
    return (Compiler*)MatchAllCompiler_new(self, searcher, boost);
}

/**********************************************************************/

MatchAllCompiler*
MatchAllCompiler_new(MatchAllQuery *parent, Searcher *searcher, 
                     float boost)
{
    MatchAllCompiler *self 
        = (MatchAllCompiler*)VTable_Make_Obj(MATCHALLCOMPILER);
    return MatchAllCompiler_init(self, parent, searcher, boost);
}

MatchAllCompiler*
MatchAllCompiler_init(MatchAllCompiler *self, MatchAllQuery *parent, 
                      Searcher *searcher, float boost)
{
    return (MatchAllCompiler*)Compiler_init((Compiler*)self, 
        (Query*)parent, searcher, NULL, boost);
}

MatchAllCompiler*
MatchAllCompiler_deserialize(MatchAllCompiler *self, InStream *instream)
{
    self = self 
         ? self 
         : (MatchAllCompiler*)VTable_Make_Obj(MATCHALLCOMPILER);
    return (MatchAllCompiler*)Compiler_deserialize((Compiler*)self, instream);
}

Matcher*
MatchAllCompiler_make_matcher(MatchAllCompiler *self, SegReader *reader,
                              bool_t need_score)
{
    float weight = MatchAllCompiler_Get_Weight(self);
    UNUSED_VAR(need_score);
    return (Matcher*)MatchAllScorer_new(weight, SegReader_Doc_Max(reader));
}


