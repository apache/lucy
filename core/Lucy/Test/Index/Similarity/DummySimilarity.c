#define C_LUCY_DUMMYSIMILARITY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test/Index/Similarity/DummySimilarity.h"

DummySimilarity*
DummySim_new(int32_t id)
{
    DummySimilarity *self
        = (DummySimilarity*)VTable_Make_Obj(DUMMYSIMILARITY);
    self->id = id;
    return (DummySimilarity*)Sim_init((Similarity*)self);
}

bool_t
DummySim_equals(DummySimilarity *self, Obj *other)
{
    if (!other) { return false; }
    DummySimilarity *evil_twin = (DummySimilarity*)other;
    if (evil_twin == self) { return true; }
    if (!Obj_Is_A(other, DUMMYSIMILARITY)) { return false; }
    if (evil_twin->id != self->id) { return false; }
    DummySim_equals_t super_equals 
        = (DummySim_equals_t)SUPER_METHOD(DUMMYSIMILARITY, DummySim, Equals);
    return super_equals(self, other);
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

