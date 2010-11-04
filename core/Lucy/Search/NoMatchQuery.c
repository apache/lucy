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
#include "Lucy/Search/NoMatchScorer.h"
#include "Lucy/Search/Searcher.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

NoMatchQuery*
NoMatchQuery_new()
{
    NoMatchQuery *self = (NoMatchQuery*)VTable_Make_Obj(NOMATCHQUERY);
    return NoMatchQuery_init(self);
}

NoMatchQuery*
NoMatchQuery_init(NoMatchQuery *self)
{
    return (NoMatchQuery*)Query_init((Query*)self, 0.0f);
}

bool_t
NoMatchQuery_equals(NoMatchQuery *self, Obj *other)
{
    NoMatchQuery *evil_twin = (NoMatchQuery*)other;
    if (!Obj_Is_A(other, NOMATCHQUERY)) return false;
    if (self->boost != evil_twin->boost) return false;
    return true;
}

CharBuf*
NoMatchQuery_to_string(NoMatchQuery *self)
{
    UNUSED_VAR(self);
    return CB_new_from_trusted_utf8("[NOMATCH]", 9);
}

Compiler*
NoMatchQuery_make_compiler(NoMatchQuery *self, Searcher *searcher, 
                            float boost)
{
    return (Compiler*)NoMatchCompiler_new(self, searcher, boost);
}

/**********************************************************************/

NoMatchCompiler*
NoMatchCompiler_new(NoMatchQuery *parent, Searcher *searcher, 
                     float boost)
{
    NoMatchCompiler *self 
        = (NoMatchCompiler*)VTable_Make_Obj(NOMATCHCOMPILER);
    return NoMatchCompiler_init(self, parent, searcher, boost);
}

NoMatchCompiler*
NoMatchCompiler_init(NoMatchCompiler *self, NoMatchQuery *parent, 
                      Searcher *searcher, float boost)
{
    return (NoMatchCompiler*)Compiler_init((Compiler*)self, 
        (Query*)parent, searcher, NULL, boost);
}

NoMatchCompiler*
NoMatchCompiler_deserialize(NoMatchCompiler *self, InStream *instream)
{
    self = self ? self : (NoMatchCompiler*)VTable_Make_Obj(NOMATCHCOMPILER);
    return (NoMatchCompiler*)Compiler_deserialize((Compiler*)self, instream);
}

Matcher*
NoMatchCompiler_make_matcher(NoMatchCompiler *self, SegReader *reader, 
                             bool_t need_score)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(need_score);
    return (Matcher*)NoMatchScorer_new();
}


