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
NumSortCache_init(NumericSortCache *self, const CharBuf *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    // Validate.
    if (!type || !FType_Sortable(type) || !FType_Is_A(type, NUMERICTYPE)) {
        DECREF(self);
        THROW(ERR, "'%o' isn't a sortable NumericType field", field);
    }

    // Mmap ords and super-init.
    int64_t  ord_len = InStream_Length(ord_in);
    void    *ords    = InStream_Buf(ord_in, (size_t)ord_len);
    SortCache_init((SortCache*)self, field, type, ords, cardinality, doc_max,
                   null_ord, ord_width);

    // Assign.
    self->ord_in = (InStream*)INCREF(ord_in);
    self->dat_in = (InStream*)INCREF(dat_in);

    // Validate ord file length.
    double BITS_PER_BYTE = 8.0;
    double docs_per_byte = BITS_PER_BYTE / self->ord_width;
    double max_ords      = ord_len * docs_per_byte;
    if (max_ords < self->doc_max + 1) {
        DECREF(self);
        THROW(ERR, "Conflict between ord count max %f64 and doc_max %i32 for "
              "field %o", max_ords, self->doc_max, field);
    }

    ABSTRACT_CLASS_CHECK(self, NUMERICSORTCACHE);
    return self;
}

void
NumSortCache_destroy(NumericSortCache *self) {
    if (self->ord_in) {
        InStream_Close(self->ord_in);
        InStream_Dec_RefCount(self->ord_in);
    }
    if (self->dat_in) {
        InStream_Close(self->dat_in);
        InStream_Dec_RefCount(self->dat_in);
    }
    SUPER_DESTROY(self, NUMERICSORTCACHE);
}

/***************************************************************************/

Float64SortCache*
F64SortCache_new(const CharBuf *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Float64SortCache *self
        = (Float64SortCache*)VTable_Make_Obj(FLOAT64SORTCACHE);
    return F64SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Float64SortCache*
F64SortCache_init(Float64SortCache *self, const CharBuf *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
F64SortCache_value(Float64SortCache *self, int32_t ord, Obj *blank) {
    if (ord == self->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", self->field, ord);
    }
    else {
        Float64 *num_blank = (Float64*)CERTIFY(blank, FLOAT64);
        InStream_Seek(self->dat_in, ord * sizeof(double));
        Float64_Set_Value(num_blank, InStream_Read_F64(self->dat_in));
    }
    return blank;
}

Float64*
F64SortCache_make_blank(Float64SortCache *self) {
    UNUSED_VAR(self);
    return Float64_new(0.0);
}

/***************************************************************************/

Float32SortCache*
F32SortCache_new(const CharBuf *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Float32SortCache *self
        = (Float32SortCache*)VTable_Make_Obj(FLOAT32SORTCACHE);
    return F32SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Float32SortCache*
F32SortCache_init(Float32SortCache *self, const CharBuf *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
F32SortCache_value(Float32SortCache *self, int32_t ord, Obj *blank) {
    if (ord == self->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", self->field, ord);
    }
    else {
        Float32 *num_blank = (Float32*)CERTIFY(blank, FLOAT32);
        InStream_Seek(self->dat_in, ord * sizeof(float));
        Float32_Set_Value(num_blank, InStream_Read_F32(self->dat_in));
    }
    return blank;
}

Float32*
F32SortCache_make_blank(Float32SortCache *self) {
    UNUSED_VAR(self);
    return Float32_new(0.0f);
}

/***************************************************************************/

Int32SortCache*
I32SortCache_new(const CharBuf *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Int32SortCache *self
        = (Int32SortCache*)VTable_Make_Obj(INT32SORTCACHE);
    return I32SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Int32SortCache*
I32SortCache_init(Int32SortCache *self, const CharBuf *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
I32SortCache_value(Int32SortCache *self, int32_t ord, Obj *blank) {
    if (ord == self->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", self->field, ord);
    }
    else {
        Integer32 *int_blank = (Integer32*)CERTIFY(blank, INTEGER32);
        InStream_Seek(self->dat_in, ord * sizeof(int32_t));
        Int32_Set_Value(int_blank, InStream_Read_I32(self->dat_in));
    }
    return blank;
}

Integer32*
I32SortCache_make_blank(Int32SortCache *self) {
    UNUSED_VAR(self);
    return Int32_new(0);
}

/***************************************************************************/

Int64SortCache*
I64SortCache_new(const CharBuf *field, FieldType *type, int32_t cardinality,
                 int32_t doc_max, int32_t null_ord, int32_t ord_width,
                 InStream *ord_in, InStream *dat_in) {
    Int64SortCache *self
        = (Int64SortCache*)VTable_Make_Obj(INT64SORTCACHE);
    return I64SortCache_init(self, field, type, cardinality, doc_max,
                             null_ord, ord_width, ord_in, dat_in);
}

Int64SortCache*
I64SortCache_init(Int64SortCache *self, const CharBuf *field,
                  FieldType *type, int32_t cardinality, int32_t doc_max,
                  int32_t null_ord, int32_t ord_width, InStream *ord_in,
                  InStream *dat_in) {
    NumSortCache_init((NumericSortCache*)self, field, type, cardinality,
                      doc_max, null_ord, ord_width, ord_in, dat_in);
    return self;
}

Obj*
I64SortCache_value(Int64SortCache *self, int32_t ord, Obj *blank) {
    if (ord == self->null_ord) {
        return NULL;
    }
    else if (ord < 0) {
        THROW(ERR, "Ordinal less than 0 for %o: %i32", self->field, ord);
    }
    else {
        Integer64 *int_blank = (Integer64*)CERTIFY(blank, INTEGER64);
        InStream_Seek(self->dat_in, ord * sizeof(int64_t));
        Int64_Set_Value(int_blank, InStream_Read_I64(self->dat_in));
    }
    return blank;
}

Integer64*
I64SortCache_make_blank(Int64SortCache *self) {
    UNUSED_VAR(self);
    return Int64_new(0);
}


