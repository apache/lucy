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

#define C_LUCY_SPAN
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Search/Span.h"

Span*
Span_new(int32_t offset, int32_t length, float weight) {
    Span *self = (Span*)Class_Make_Obj(SPAN);
    return Span_init(self, offset, length, weight);
}

Span*
Span_init(Span *self, int32_t offset, int32_t length,
          float weight) {
    SpanIVARS *const ivars = Span_IVARS(self);
    ivars->offset   = offset;
    ivars->length   = length;
    ivars->weight   = weight;
    return self;
}

int32_t
Span_Get_Offset_IMP(Span *self) {
    return Span_IVARS(self)->offset;
}

int32_t
Span_Get_Length_IMP(Span *self) {
    return Span_IVARS(self)->length;
}

float
Span_Get_Weight_IMP(Span *self) {
    return Span_IVARS(self)->weight;
}

void
Span_Set_Offset_IMP(Span *self, int32_t offset) {
    Span_IVARS(self)->offset = offset;
}

void
Span_Set_Length_IMP(Span *self, int32_t length) {
    Span_IVARS(self)->length = length;
}

void
Span_Set_Weight_IMP(Span *self, float weight) {
    Span_IVARS(self)->weight = weight;
}

bool
Span_Equals_IMP(Span *self, Obj *other) {
    if (self == (Span*)other)         { return true; }
    if (!Obj_Is_A(other, SPAN))       { return false; }
    SpanIVARS *const ivars = Span_IVARS(self);
    SpanIVARS *const ovars = Span_IVARS((Span*)other);
    if (ivars->offset != ovars->offset) { return false; }
    if (ivars->length != ovars->length) { return false; }
    if (ivars->weight != ovars->weight) { return false; }
    return true;
}

int32_t
Span_Compare_To_IMP(Span *self, Obj *other) {
    CERTIFY(other, SPAN);
    SpanIVARS *const ivars = Span_IVARS(self);
    SpanIVARS *const ovars = Span_IVARS((Span*)other);
    int32_t comparison = ivars->offset - ovars->offset;
    if (comparison == 0) { comparison = ivars->length - ovars->length; }
    return comparison;
}


