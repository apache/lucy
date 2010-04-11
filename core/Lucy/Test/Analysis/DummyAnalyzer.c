#define C_LUCY_DUMMYANALYZER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Analysis/DummyAnalyzer.h"

DummyAnalyzer*
DummyAnalyzer_new(int32_t id)
{
    DummyAnalyzer *self = (DummyAnalyzer*)VTable_Make_Obj(DUMMYANALYZER);
    return DummyAnalyzer_init(self, id);
}

DummyAnalyzer*
DummyAnalyzer_init(DummyAnalyzer *self, int32_t id)
{
    Analyzer_init((Analyzer*)self);
    self->id = id;
    return self;
}

bool_t
DummyAnalyzer_equals(DummyAnalyzer *self, Obj *other)
{
    DummyAnalyzer *evil_twin = (DummyAnalyzer*)other;
    if (!Obj_Is_A(other, DUMMYANALYZER)) return false;
    if (self->id != evil_twin->id) return false;
    return true;
}

/* Copyright 2010 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

