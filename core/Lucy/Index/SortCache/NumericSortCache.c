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

#define C_LUCY_NUMERICSORTCACHE
#define C_LUCY_INT32SORTCACHE
#define C_LUCY_INT64SORTCACHE
#define C_LUCY_FLOAT32SORTCACHE
#define C_LUCY_FLOAT64SORTCACHE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SortCache/NumericSortCache.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"

NumericSortCache*
NumSortCache_init(NumericSortCache *self, String *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    // Validate.
    if (!type || !FType_Sortable(type) || !FType_Is_A(type, NUMERICTYPE)) {
        DECREF(self);
        THROW(ERR, "'%o' isn't a sortable NumericType field", field);
    }

    // Mmap ords and super-init.
    int64_t     ord_len = InStream_Length(ord_in);
    const void *ords    = InStream_Buf(ord_in, (size_t)ord_len);
    SortCache_init((SortCache*)self, field, type, ords, cardinality, doc_max,
                   null_ord, ord_width);
    NumericSortCacheIVARS *const ivars = NumSortCache_IVARS(self);

    // Assign.
    ivars->ord_in = (InStream*)INCREF(ord_in);
    ivars->dat_in = (InStream*)INCREF(dat_in);

    // Validate ord file length.
    double BITS_PER_BYTE = 8.0;
    double docs_per_byte = BITS_PER_BYTE / ivars->ord_width;
    double max_ords      = ord_len * docs_per_byte;
    if (max_ords < ivars->doc_max + 1) {
        DECREF(self);
        THROW(ERR, "Conflict between ord count max %f64 and doc_max %i32 for "
              "field %o", max_ords, ivars->doc_max, field);
    }

    ABSTRACT_CLASS_CHECK(self, NUMERICSORTCACHE);
    return self;
}

void
NumSortCache_Destroy_IMP(NumericSortCache *self) {
    NumericSortCacheIVARS *const ivars = NumSortCache_IVARS(self);
    if (ivars->ord_in) {
        InStream_Close(ivars->ord_in);
        DECREF(ivars->ord_in);
    }
    if (ivars->dat_in) {
        InStream_Close(ivars->dat_in);
        DECREF(ivars->dat_in);
    }
    SUPER_DESTROY(self, NUMERICSORTCACHE);
}

/***************************************************************************/

Float64SortCache*
F64SortCache_new(String *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Float64SortCache *self
        = (Float64SortCache*)Class_Make_Obj(FLOAT64SORTCACHE);
    return F64SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Float64SortCache*
F64SortCache_init(Float64SortCache *self, String *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
F64SortCache_Value_IMP(Float64SortCache *self, int32_t ord) {
    Float64SortCacheIVARS *const ivars = F64SortCache_IVARS(self);
    if (ord == ivars->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", ivars->field, ord);
        UNREACHABLE_RETURN(Obj*);
    }
    else {
        InStream_Seek(ivars->dat_in, ord * sizeof(double));
        return (Obj*)Float64_new(InStream_Read_F64(ivars->dat_in));
    }
}

/***************************************************************************/

Float32SortCache*
F32SortCache_new(String *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Float32SortCache *self
        = (Float32SortCache*)Class_Make_Obj(FLOAT32SORTCACHE);
    return F32SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Float32SortCache*
F32SortCache_init(Float32SortCache *self, String *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
F32SortCache_Value_IMP(Float32SortCache *self, int32_t ord) {
    Float32SortCacheIVARS *const ivars = F32SortCache_IVARS(self);
    if (ord == ivars->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", ivars->field, ord);
        UNREACHABLE_RETURN(Obj*);
    }
    else {
        InStream_Seek(ivars->dat_in, ord * sizeof(float));
        return (Obj*)Float32_new(InStream_Read_F32(ivars->dat_in));
    }
}

/***************************************************************************/

Int32SortCache*
I32SortCache_new(String *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Int32SortCache *self
        = (Int32SortCache*)Class_Make_Obj(INT32SORTCACHE);
    return I32SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Int32SortCache*
I32SortCache_init(Int32SortCache *self, String *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
I32SortCache_Value_IMP(Int32SortCache *self, int32_t ord) {
    Int32SortCacheIVARS *const ivars = I32SortCache_IVARS(self);
    if (ord == ivars->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", ivars->field, ord);
        UNREACHABLE_RETURN(Obj*);
    }
    else {
        InStream_Seek(ivars->dat_in, ord * sizeof(int32_t));
        return (Obj*)Int32_new(InStream_Read_I32(ivars->dat_in));
    }
}

/***************************************************************************/

Int64SortCache*
I64SortCache_new(String *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Int64SortCache *self
        = (Int64SortCache*)Class_Make_Obj(INT64SORTCACHE);
    return I64SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Int64SortCache*
I64SortCache_init(Int64SortCache *self, String *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
I64SortCache_Value_IMP(Int64SortCache *self, int32_t ord) {
    Int64SortCacheIVARS *const ivars = I64SortCache_IVARS(self);
    if (ord == ivars->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", ivars->field, ord);
        UNREACHABLE_RETURN(Obj*);
    }
    else {
        InStream_Seek(ivars->dat_in, ord * sizeof(int64_t));
        return (Obj*)Int64_new(InStream_Read_I64(ivars->dat_in));
    }
}


