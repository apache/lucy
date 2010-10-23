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

#define C_KINO_NOMATCHSCORER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/NoMatchScorer.h"
#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"

NoMatchScorer*
NoMatchScorer_new()
{
    NoMatchScorer *self = (NoMatchScorer*)VTable_Make_Obj(NOMATCHSCORER);
    return NoMatchScorer_init(self);
}

NoMatchScorer*
NoMatchScorer_init(NoMatchScorer *self)
{
    return (NoMatchScorer*)Matcher_init((Matcher*)self);
}   

int32_t
NoMatchScorer_next(NoMatchScorer* self) 
{
    UNUSED_VAR(self);
    return 0;
}

int32_t
NoMatchScorer_advance(NoMatchScorer* self, int32_t target) 
{
    UNUSED_VAR(self);
    UNUSED_VAR(target);
    return 0;
}


