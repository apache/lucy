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

#define C_LUCY_SORTREADER
#define C_LUCY_DEFAULTSORTREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SortReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SortCache/NumericSortCache.h"
#include "Lucy/Index/SortCache/TextSortCache.h"
#include "Lucy/Index/SortWriter.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"

SortReader*
SortReader_init(SortReader *self, Schema *schema, Folder *folder,
                Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    ABSTRACT_CLASS_CHECK(self, SORTREADER);
    return self;
}

DataReader*
SortReader_aggregator(SortReader *self, VArray *readers, I32Array *offsets) {
    UNUSED_VAR(self);
    UNUSED_VAR(readers);
    UNUSED_VAR(offsets);
    return NULL;
}

DefaultSortReader*
DefSortReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                  VArray *segments, int32_t seg_tick) {
    DefaultSortReader *self
        = (DefaultSortReader*)VTable_Make_Obj(DEFAULTSORTREADER);
    return DefSortReader_init(self, schema, folder, snapshot, segments,
                              seg_tick);
}

DefaultSortReader*
DefSortReader_init(DefaultSortReader *self, Schema *schema, Folder *folder,
                   Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    Segment *segment;
    Hash    *metadata;
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    segment = DefSortReader_Get_Segment(self);
    metadata = (Hash*)Seg_Fetch_Metadata_Str(segment, "sort", 4);

    // Check format.
    self->format = 0;
    if (metadata) {
        Obj *format = Hash_Fetch_Str(metadata, "format", 6);
        if (!format) { THROW(ERR, "Missing 'format' var"); }
        else {
            self->format = (int32_t)Obj_To_I64(format);
            if (self->format < 2 || self->format > 3) {
                THROW(ERR, "Unsupported sort cache format: %i32",
                      self->format);
            }
        }
    }

    // Init.
    self->caches = Hash_new(0);

    // Either extract or fake up the "counts", "null_ords", and "ord_widths"
    // hashes.
    if (metadata) {
        self->counts
            = (Hash*)INCREF(CERTIFY(Hash_Fetch_Str(metadata, "counts", 6),
                                    HASH));
        self->null_ords = (Hash*)Hash_Fetch_Str(metadata, "null_ords", 9);
        if (self->null_ords) {
            CERTIFY(self->null_ords, HASH);
            INCREF(self->null_ords);
        }
        else {
            self->null_ords = Hash_new(0);
        }
        self->ord_widths = (Hash*)Hash_Fetch_Str(metadata, "ord_widths", 10);
        if (self->ord_widths) {
            CERTIFY(self->ord_widths, HASH);
            INCREF(self->ord_widths);
        }
        else {
            self->ord_widths = Hash_new(0);
        }
    }
    else {
        self->counts     = Hash_new(0);
        self->null_ords  = Hash_new(0);
        self->ord_widths = Hash_new(0);
    }

    return self;
}

void
DefSortReader_close(DefaultSortReader *self) {
    if (self->caches) {
        Hash_Dec_RefCount(self->caches);
        self->caches = NULL;
    }
    if (self->counts) {
        Hash_Dec_RefCount(self->counts);
        self->counts = NULL;
    }
    if (self->null_ords) {
        Hash_Dec_RefCount(self->null_ords);
        self->null_ords = NULL;
    }
    if (self->ord_widths) {
        Hash_Dec_RefCount(self->ord_widths);
        self->ord_widths = NULL;
    }
}

void
DefSortReader_destroy(DefaultSortReader *self) {
    DECREF(self->caches);
    DECREF(self->counts);
    DECREF(self->null_ords);
    DECREF(self->ord_widths);
    SUPER_DESTROY(self, DEFAULTSORTREADER);
}

static int32_t
S_calc_ord_width(int32_t cardinality) {
    if (cardinality <= 0x00000002)      { return 1; }
    else if (cardinality <= 0x00000004) { return 2; }
    else if (cardinality <= 0x0000000F) { return 4; }
    else if (cardinality <= 0x000000FF) { return 8; }
    else if (cardinality <= 0x0000FFFF) { return 16; }
    else                                { return 32; }
}

static SortCache*
S_lazy_init_sort_cache(DefaultSortReader *self, const CharBuf *field) {
    // See if we have any values.
    Obj *count_obj = Hash_Fetch(self->counts, (Obj*)field);
    int32_t count = count_obj ? (int32_t)Obj_To_I64(count_obj) : 0;
    if (!count) { return NULL; }

    // Get a FieldType and sanity check that the field is sortable.
    Schema    *schema = DefSortReader_Get_Schema(self);
    FieldType *type   = Schema_Fetch_Type(schema, field);
    if (!type || !FType_Sortable(type)) {
        THROW(ERR, "'%o' isn't a sortable field", field);
    }

    // Open streams.
    Folder    *folder    = DefSortReader_Get_Folder(self);
    Segment   *segment   = DefSortReader_Get_Segment(self);
    CharBuf   *seg_name  = Seg_Get_Name(segment);
    CharBuf   *path      = CB_new(40);
    int32_t    field_num = Seg_Field_Num(segment, field);
    int8_t     prim_id   = FType_Primitive_ID(type);
    bool_t     var_width = (prim_id == FType_TEXT || prim_id == FType_BLOB)
                           ? true
                           : false;
    CB_setf(path, "%o/sort-%i32.ord", seg_name, field_num);
    InStream *ord_in = Folder_Open_In(folder, path);
    if (!ord_in) {
        DECREF(path);
        THROW(ERR, "Error building sort cache for '%o': %o",
              field, Err_get_error());
    }
    InStream *ix_in = NULL;
    if (var_width) {
        CB_setf(path, "%o/sort-%i32.ix", seg_name, field_num);
        ix_in = Folder_Open_In(folder, path);
        if (!ix_in) {
            DECREF(path);
            THROW(ERR, "Error building sort cache for '%o': %o",
                  field, Err_get_error());
        }
    }
    CB_setf(path, "%o/sort-%i32.dat", seg_name, field_num);
    InStream *dat_in = Folder_Open_In(folder, path);
    if (!dat_in) {
        DECREF(path);
        THROW(ERR, "Error building sort cache for '%o': %o",
              field, Err_get_error());
    }
    DECREF(path);

    Obj     *null_ord_obj = Hash_Fetch(self->null_ords, (Obj*)field);
    int32_t  null_ord = null_ord_obj ? (int32_t)Obj_To_I64(null_ord_obj) : -1;
    Obj     *ord_width_obj = Hash_Fetch(self->ord_widths, (Obj*)field);
    int32_t  ord_width = ord_width_obj
                         ? (int32_t)Obj_To_I64(ord_width_obj)
                         : S_calc_ord_width(count);
    int32_t  doc_max = (int32_t)Seg_Get_Count(segment);

    SortCache *cache = NULL;
    switch (prim_id & FType_PRIMITIVE_ID_MASK) {
        case FType_TEXT:
            cache = (SortCache*)TextSortCache_new(field, type, count, doc_max,
                                                  null_ord, ord_width, ord_in,
                                                  ix_in, dat_in);
            break;
        case FType_INT32:
            cache = (SortCache*)I32SortCache_new(field, type, count, doc_max,
                                                 null_ord, ord_width, ord_in,
                                                 dat_in);
            break;
        case FType_INT64:
            cache = (SortCache*)I64SortCache_new(field, type, count, doc_max,
                                                 null_ord, ord_width, ord_in,
                                                 dat_in);
            break;
        case FType_FLOAT32:
            cache = (SortCache*)F32SortCache_new(field, type, count, doc_max,
                                                 null_ord, ord_width, ord_in,
                                                 dat_in);
            break;
        case FType_FLOAT64:
            cache = (SortCache*)F64SortCache_new(field, type, count, doc_max,
                                                 null_ord, ord_width, ord_in,
                                                 dat_in);
            break;
        default:
            THROW(ERR, "No SortCache class for %o", type);
    }
    Hash_Store(self->caches, (Obj*)field, (Obj*)cache);

    if (self->format == 2) { // bug compatibility
        SortCache_Set_Native_Ords(cache, true);
    }

    DECREF(ord_in);
    DECREF(ix_in);
    DECREF(dat_in);

    return cache;
}

SortCache*
DefSortReader_fetch_sort_cache(DefaultSortReader *self, const CharBuf *field) {
    SortCache *cache = NULL;

    if (field) {
        cache = (SortCache*)Hash_Fetch(self->caches, (Obj*)field);
        if (!cache) {
            cache = S_lazy_init_sort_cache(self, field);
        }
    }

    return cache;
}


