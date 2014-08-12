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

#define C_LUCY_TEXTSORTCACHE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SortCache/TextSortCache.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/Folder.h"

TextSortCache*
TextSortCache_new(String *field, FieldType *type, int32_t cardinality,
                  int32_t doc_max, int32_t null_ord, int32_t ord_width,
                  InStream *ord_in, InStream *ix_in, InStream *dat_in) {
    TextSortCache *self = (TextSortCache*)Class_Make_Obj(TEXTSORTCACHE);
    return TextSortCache_init(self, field, type, cardinality, doc_max,
                              null_ord, ord_width, ord_in, ix_in, dat_in);
}

TextSortCache*
TextSortCache_init(TextSortCache *self, String *field,
                   FieldType *type, int32_t cardinality,
                   int32_t doc_max, int32_t null_ord, int32_t ord_width,
                   InStream *ord_in, InStream *ix_in, InStream *dat_in) {
    // Validate.
    if (!type || !FType_Sortable(type)) {
        DECREF(self);
        THROW(ERR, "'%o' isn't a sortable field", field);
    }

    // Memory map ords and super-init.
    int64_t ord_len = InStream_Length(ord_in);
    const void *ords = InStream_Buf(ord_in, (size_t)ord_len);
    SortCache_init((SortCache*)self, field, type, ords, cardinality, doc_max,
                   null_ord, ord_width);
    TextSortCacheIVARS *const ivars = TextSortCache_IVARS(self);

    // Validate ords file length.
    double  bytes_per_doc = ivars->ord_width / 8.0;
    double  max_ords      = ord_len / bytes_per_doc;
    if (max_ords < ivars->doc_max + 1) {
        WARN("ORD WIDTH: %i32 %i32", ord_width, ivars->ord_width);
        THROW(ERR, "Conflict between ord count max %f64 and doc_max %i32 for "
              "field %o", max_ords, doc_max, field);
    }

    // Assign.
    ivars->ord_in = (InStream*)INCREF(ord_in);
    ivars->ix_in  = (InStream*)INCREF(ix_in);
    ivars->dat_in = (InStream*)INCREF(dat_in);

    return self;
}

void
TextSortCache_Destroy_IMP(TextSortCache *self) {
    TextSortCacheIVARS *const ivars = TextSortCache_IVARS(self);
    if (ivars->ord_in) {
        InStream_Close(ivars->ord_in);
        DECREF(ivars->ord_in);
    }
    if (ivars->ix_in) {
        InStream_Close(ivars->ix_in);
        DECREF(ivars->ix_in);
    }
    if (ivars->dat_in) {
        InStream_Close(ivars->dat_in);
        DECREF(ivars->dat_in);
    }
    SUPER_DESTROY(self, TEXTSORTCACHE);
}

#define NULL_SENTINEL -1

Obj*
TextSortCache_Value_IMP(TextSortCache *self, int32_t ord) {
    TextSortCacheIVARS *const ivars = TextSortCache_IVARS(self);
    if (ord == ivars->null_ord) {
        return NULL;
    }
    InStream_Seek(ivars->ix_in, ord * sizeof(int64_t));
    int64_t offset = InStream_Read_I64(ivars->ix_in);
    if (offset == NULL_SENTINEL) {
        return NULL;
    }
    else {
        uint32_t next_ord = ord + 1;
        int64_t next_offset;
        while (1) {
            InStream_Seek(ivars->ix_in, next_ord * sizeof(int64_t));
            next_offset = InStream_Read_I64(ivars->ix_in);
            if (next_offset != NULL_SENTINEL) { break; }
            next_ord++;
        }

        // Read character data into String.
        size_t len = (size_t)(next_offset - offset);
        char *ptr = (char*)MALLOCATE(len + 1);
        InStream_Seek(ivars->dat_in, offset);
        InStream_Read_Bytes(ivars->dat_in, ptr, len);
        ptr[len] = '\0';
        return (Obj*)Str_new_steal_utf8(ptr, len);
    }
}


