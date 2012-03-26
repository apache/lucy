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

#define C_LUCY_SORTCACHE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SortCache.h"
#include "Lucy/Plan/FieldType.h"

SortCache*
SortCache_init(SortCache *self, const CharBuf *field, FieldType *type,
               void *ords, int32_t cardinality, int32_t doc_max, int32_t null_ord,
               int32_t ord_width) {
    // Init.
    self->native_ords = false;

    // Assign.
    if (!FType_Sortable(type)) {
        THROW(ERR, "Non-sortable FieldType for %o", field);
    }
    self->field       = CB_Clone(field);
    self->type        = (FieldType*)INCREF(type);
    self->ords        = ords;
    self->cardinality = cardinality;
    self->doc_max     = doc_max;
    self->null_ord    = null_ord;
    self->ord_width   = ord_width;

    ABSTRACT_CLASS_CHECK(self, SORTCACHE);
    return self;
}

void
SortCache_destroy(SortCache *self) {
    DECREF(self->field);
    DECREF(self->type);
    SUPER_DESTROY(self, SORTCACHE);
}

bool_t
SortCache_get_native_ords(SortCache *self) {
    return self->native_ords;
}

void
SortCache_set_native_ords(SortCache *self, bool_t native_ords) {
    self->native_ords = native_ords;
}

int32_t
SortCache_ordinal(SortCache *self, int32_t doc_id) {
    if ((uint32_t)doc_id > (uint32_t)self->doc_max) {
        THROW(ERR, "Out of range: %i32 > %i32", doc_id, self->doc_max);
    }
    switch (self->ord_width) {
        case 1: return NumUtil_u1get(self->ords, doc_id);
        case 2: return NumUtil_u2get(self->ords, doc_id);
        case 4: return NumUtil_u4get(self->ords, doc_id);
        case 8: {
                uint8_t *ints = (uint8_t*)self->ords;
                return ints[doc_id];
            }
        case 16:
            if (self->native_ords) {
                uint16_t *ints = (uint16_t*)self->ords;
                return ints[doc_id];
            }
            else {
                uint8_t *bytes = (uint8_t*)self->ords;
                bytes += doc_id * sizeof(uint16_t);
                return NumUtil_decode_bigend_u16(bytes);
            }
        case 32:
            if (self->native_ords) {
                uint32_t *ints = (uint32_t*)self->ords;
                return ints[doc_id];
            }
            else {
                uint8_t *bytes = (uint8_t*)self->ords;
                bytes += doc_id * sizeof(uint32_t);
                return NumUtil_decode_bigend_u32(bytes);
            }
        default: {
                THROW(ERR, "Invalid ord width: %i32", self->ord_width);
                UNREACHABLE_RETURN(int32_t);
            }
    }
}

int32_t
SortCache_find(SortCache *self, Obj *term) {
    FieldType *const type   = self->type;
    int32_t          lo     = 0;
    int32_t          hi     = self->cardinality - 1;
    int32_t          result = -100;
    Obj             *blank  = SortCache_Make_Blank(self);

    if (term != NULL
        && !Obj_Is_A(term, Obj_Get_VTable(blank))
        && !Obj_Is_A(blank, Obj_Get_VTable(term))
       ) {
        THROW(ERR, "SortCache error for field %o: term is a %o, and not "
              "comparable to a %o", self->field, Obj_Get_Class_Name(term),
              Obj_Get_Class_Name(blank));
    }

    // Binary search.
    while (hi >= lo) {
        const int32_t mid = lo + ((hi - lo) / 2);
        Obj *val = SortCache_Value(self, mid, blank);
        int32_t comparison = FType_null_back_compare_values(type, term, val);
        if (comparison < 0) {
            hi = mid - 1;
        }
        else if (comparison > 0) {
            lo = mid + 1;
        }
        else {
            result = mid;
            break;
        }
    }

    DECREF(blank);

    if (hi < 0) {
        // Target is "less than" the first cache entry.
        return -1;
    }
    else if (result == -100) {
        // If result is still -100, it wasn't set.
        return hi;
    }
    else {
        return result;
    }
}

void*
SortCache_get_ords(SortCache *self) {
    return self->ords;
}

int32_t
SortCache_get_cardinality(SortCache *self) {
    return self->cardinality;
}

int32_t
SortCache_get_null_ord(SortCache *self) {
    return self->null_ord;
}

int32_t
SortCache_get_ord_width(SortCache *self) {
    return self->ord_width;
}


