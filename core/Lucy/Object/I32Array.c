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

#define C_LUCY_I32ARRAY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Object/I32Array.h"

I32Array*
I32Arr_new(int32_t *ints, uint32_t size) {
    I32Array *self = (I32Array*)Class_Make_Obj(I32ARRAY);
    int32_t *ints_copy = (int32_t*)MALLOCATE(size * sizeof(int32_t));
    memcpy(ints_copy, ints, size * sizeof(int32_t));
    return I32Arr_init(self, ints_copy, size);
}

I32Array*
I32Arr_new_blank(uint32_t size) {
    I32Array *self = (I32Array*)Class_Make_Obj(I32ARRAY);
    int32_t *ints = (int32_t*)CALLOCATE(size, sizeof(int32_t));
    return I32Arr_init(self, ints, size);
}

I32Array*
I32Arr_new_steal(int32_t *ints, uint32_t size) {
    I32Array *self = (I32Array*)Class_Make_Obj(I32ARRAY);
    return I32Arr_init(self, ints, size);
}

I32Array*
I32Arr_init(I32Array *self, int32_t *ints, uint32_t size) {
    I32ArrayIVARS *const ivars = I32Arr_IVARS(self);
    ivars->ints = ints;
    ivars->size = size;
    return self;
}

void
I32Arr_Destroy_IMP(I32Array *self) {
    I32ArrayIVARS *const ivars = I32Arr_IVARS(self);
    FREEMEM(ivars->ints);
    SUPER_DESTROY(self, I32ARRAY);
}

void
I32Arr_Set_IMP(I32Array *self, uint32_t tick, int32_t value) {
    I32ArrayIVARS *const ivars = I32Arr_IVARS(self);
    if (tick >= ivars->size) {
        THROW(ERR, "Out of bounds: %u32 >= %u32", tick, ivars->size);
    }
    ivars->ints[tick] = value;
}

int32_t
I32Arr_Get_IMP(I32Array *self, uint32_t tick) {
    I32ArrayIVARS *const ivars = I32Arr_IVARS(self);
    if (tick >= ivars->size) {
        THROW(ERR, "Out of bounds: %u32 >= %u32", tick, ivars->size);
    }
    return ivars->ints[tick];
}

uint32_t
I32Arr_Get_Size_IMP(I32Array *self) {
    return I32Arr_IVARS(self)->size;
}


