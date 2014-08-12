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
SortCache_init(SortCache *self, String *field, FieldType *type,
               const void *ords, int32_t cardinality, int32_t doc_max,
               int32_t null_ord, int32_t ord_width) {
    SortCacheIVARS *const ivars = SortCache_IVARS(self);

    // Init.
    ivars->native_ords = false;

    // Assign.
    if (!FType_Sortable(type)) {
        THROW(ERR, "Non-sortable FieldType for %o", field);
    }
    ivars->field       = Str_Clone(field);
    ivars->type        = (FieldType*)INCREF(type);
    ivars->ords        = ords;
    ivars->cardinality = cardinality;
    ivars->doc_max     = doc_max;
    ivars->null_ord    = null_ord;
    ivars->ord_width   = ord_width;

    ABSTRACT_CLASS_CHECK(self, SORTCACHE);
    return self;
}

void
SortCache_Destroy_IMP(SortCache *self) {
    SortCacheIVARS *const ivars = SortCache_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->type);
    SUPER_DESTROY(self, SORTCACHE);
}

bool
SortCache_Get_Native_Ords_IMP(SortCache *self) {
    return SortCache_IVARS(self)->native_ords;
}

void
SortCache_Set_Native_Ords_IMP(SortCache *self, bool native_ords) {
    SortCache_IVARS(self)->native_ords = native_ords;
}

int32_t
SortCache_Ordinal_IMP(SortCache *self, int32_t doc_id) {
    SortCacheIVARS *const ivars = SortCache_IVARS(self);
    if ((uint32_t)doc_id > (uint32_t)ivars->doc_max) {
        THROW(ERR, "Out of range: %i32 > %i32", doc_id, ivars->doc_max);
    }
    switch (ivars->ord_width) {
        case 1: return NumUtil_u1get(ivars->ords, doc_id);
        case 2: return NumUtil_u2get(ivars->ords, doc_id);
        case 4: return NumUtil_u4get(ivars->ords, doc_id);
        case 8: {
                uint8_t *ints = (uint8_t*)ivars->ords;
                return ints[doc_id];
            }
        case 16:
            if (ivars->native_ords) {
                uint16_t *ints = (uint16_t*)ivars->ords;
                return ints[doc_id];
            }
            else {
                uint8_t *bytes = (uint8_t*)ivars->ords;
                bytes += doc_id * sizeof(uint16_t);
                return NumUtil_decode_bigend_u16(bytes);
            }
        case 32:
            if (ivars->native_ords) {
                uint32_t *ints = (uint32_t*)ivars->ords;
                return ints[doc_id];
            }
            else {
                uint8_t *bytes = (uint8_t*)ivars->ords;
                bytes += doc_id * sizeof(uint32_t);
                return NumUtil_decode_bigend_u32(bytes);
            }
        default: {
                THROW(ERR, "Invalid ord width: %i32", ivars->ord_width);
                UNREACHABLE_RETURN(int32_t);
            }
    }
}

int32_t
SortCache_Find_IMP(SortCache *self, Obj *term) {
    SortCacheIVARS *const ivars = SortCache_IVARS(self);
    FieldType *const type   = ivars->type;
    int32_t          lo     = 0;
    int32_t          hi     = ivars->cardinality - 1;
    int32_t          result = -100;

    // Binary search.
    while (hi >= lo) {
        const int32_t mid = lo + ((hi - lo) / 2);
        Obj *val = SortCache_Value(self, mid);
        int32_t comparison = FType_null_back_compare_values(type, term, val);
        DECREF(val);
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

const void*
SortCache_Get_Ords_IMP(SortCache *self) {
    return SortCache_IVARS(self)->ords;
}

int32_t
SortCache_Get_Cardinality_IMP(SortCache *self) {
    return SortCache_IVARS(self)->cardinality;
}

int32_t
SortCache_Get_Null_Ord_IMP(SortCache *self) {
    return SortCache_IVARS(self)->null_ord;
}

int32_t
SortCache_Get_Ord_Width_IMP(SortCache *self) {
    return SortCache_IVARS(self)->ord_width;
}


