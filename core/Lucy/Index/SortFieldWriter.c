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

#define C_LUCY_SORTFIELDWRITER
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Lucy/Index/SortFieldWriter.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Index/SortCache/NumericSortCache.h"
#include "Lucy/Index/SortCache/TextSortCache.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Index/ZombieKeyedHash.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Memory.h"
#include "Lucy/Util/MemoryPool.h"
#include "Lucy/Util/SortUtils.h"

// Prepare to read back a run.
static void
S_flip_run(SortFieldWriter *run, size_t sub_thresh, InStream *ord_in,
           InStream *ix_in, InStream *dat_in);

// Write out a sort cache.  Returns the number of unique values in the sort
// cache.
static int32_t
S_write_files(SortFieldWriter *self, OutStream *ord_out, OutStream *ix_out,
              OutStream *dat_out);

typedef struct lucy_SFWriterElem {
    Obj *value;
    int32_t doc_id;
} lucy_SFWriterElem;
#define SFWriterElem lucy_SFWriterElem

SortFieldWriter*
SortFieldWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                    PolyReader *polyreader, const CharBuf *field,
                    MemoryPool *memory_pool, size_t mem_thresh,
                    OutStream *temp_ord_out, OutStream *temp_ix_out,
                    OutStream *temp_dat_out) {
    SortFieldWriter *self
        = (SortFieldWriter*)VTable_Make_Obj(SORTFIELDWRITER);
    return SortFieldWriter_init(self, schema, snapshot, segment, polyreader,
                                field, memory_pool, mem_thresh, temp_ord_out,
                                temp_ix_out, temp_dat_out);
}

SortFieldWriter*
SortFieldWriter_init(SortFieldWriter *self, Schema *schema,
                     Snapshot *snapshot, Segment *segment,
                     PolyReader *polyreader, const CharBuf *field,
                     MemoryPool *memory_pool, size_t mem_thresh,
                     OutStream *temp_ord_out, OutStream *temp_ix_out,
                     OutStream *temp_dat_out) {
    // Init.
    SortEx_init((SortExternal*)self, sizeof(SFWriterElem));
    self->null_ord        = -1;
    self->count           = 0;
    self->ord_start       = 0;
    self->ord_end         = 0;
    self->ix_start        = 0;
    self->ix_end          = 0;
    self->dat_start       = 0;
    self->dat_end         = 0;
    self->run_cardinality = -1;
    self->run_max         = -1;
    self->sort_cache      = NULL;
    self->doc_map         = NULL;
    self->sorted_ids      = NULL;
    self->run_ord         = 0;
    self->run_tick        = 0;
    self->ord_width       = 0;

    // Assign.
    self->field        = CB_Clone(field);
    self->schema       = (Schema*)INCREF(schema);
    self->snapshot     = (Snapshot*)INCREF(snapshot);
    self->segment      = (Segment*)INCREF(segment);
    self->polyreader   = (PolyReader*)INCREF(polyreader);
    self->mem_pool     = (MemoryPool*)INCREF(memory_pool);
    self->temp_ord_out = (OutStream*)INCREF(temp_ord_out);
    self->temp_ix_out  = (OutStream*)INCREF(temp_ix_out);
    self->temp_dat_out = (OutStream*)INCREF(temp_dat_out);
    self->mem_thresh   = mem_thresh;

    // Derive.
    self->field_num = Seg_Field_Num(segment, field);
    FieldType *type = (FieldType*)CERTIFY(
                          Schema_Fetch_Type(self->schema, field), FIELDTYPE);
    self->type    = (FieldType*)INCREF(type);
    self->prim_id = FType_Primitive_ID(type);
    if (self->prim_id == FType_TEXT || self->prim_id == FType_BLOB) {
        self->var_width = true;
    }
    else {
        self->var_width = false;
    }
    self->uniq_vals = (Hash*)ZKHash_new(memory_pool, self->prim_id);

    return self;
}

void
SortFieldWriter_clear_cache(SortFieldWriter *self) {
    if (self->uniq_vals) {
        Hash_Clear(self->uniq_vals);
    }
    SortFieldWriter_clear_cache_t super_clear_cache
        = (SortFieldWriter_clear_cache_t)SUPER_METHOD(
              self->vtable, SortFieldWriter, Clear_Cache);
    super_clear_cache(self);
}

void
SortFieldWriter_destroy(SortFieldWriter *self) {
    DECREF(self->uniq_vals);
    self->uniq_vals = NULL;
    DECREF(self->field);
    DECREF(self->schema);
    DECREF(self->snapshot);
    DECREF(self->segment);
    DECREF(self->polyreader);
    DECREF(self->type);
    DECREF(self->mem_pool);
    DECREF(self->temp_ord_out);
    DECREF(self->temp_ix_out);
    DECREF(self->temp_dat_out);
    DECREF(self->ord_in);
    DECREF(self->ix_in);
    DECREF(self->dat_in);
    DECREF(self->sort_cache);
    DECREF(self->doc_map);
    FREEMEM(self->sorted_ids);
    SUPER_DESTROY(self, SORTFIELDWRITER);
}

int32_t
SortFieldWriter_get_null_ord(SortFieldWriter *self) {
    return self->null_ord;
}

int32_t
SortFieldWriter_get_ord_width(SortFieldWriter *self) {
    return self->ord_width;
}

static Obj*
S_find_unique_value(Hash *uniq_vals, Obj *val) {
    int32_t  hash_sum  = Obj_Hash_Sum(val);
    Obj     *uniq_val  = Hash_Find_Key(uniq_vals, val, hash_sum);
    if (!uniq_val) {
        Hash_Store(uniq_vals, val, INCREF(&EMPTY));
        uniq_val = Hash_Find_Key(uniq_vals, val, hash_sum);
    }
    return uniq_val;
}

void
SortFieldWriter_add(SortFieldWriter *self, int32_t doc_id, Obj *value) {
    // Uniq-ify the value, and record it for this document.
    SFWriterElem elem;
    elem.value = S_find_unique_value(self->uniq_vals, value);
    elem.doc_id = doc_id;
    SortFieldWriter_Feed(self, &elem);
    self->count++;
}

void
SortFieldWriter_add_segment(SortFieldWriter *self, SegReader *reader,
                            I32Array *doc_map, SortCache *sort_cache) {
    if (!sort_cache) { return; }
    SortFieldWriter *run
        = SortFieldWriter_new(self->schema, self->snapshot, self->segment,
                              self->polyreader, self->field, self->mem_pool,
                              self->mem_thresh, NULL, NULL, NULL);
    run->sort_cache = (SortCache*)INCREF(sort_cache);
    run->doc_map    = (I32Array*)INCREF(doc_map);
    run->run_max    = SegReader_Doc_Max(reader);
    run->run_cardinality = SortCache_Get_Cardinality(sort_cache);
    run->null_ord   = SortCache_Get_Null_Ord(sort_cache);
    run->run_tick   = 1;
    SortFieldWriter_Add_Run(self, (SortExternal*)run);
}

static int32_t
S_calc_width(int32_t cardinality) {
    if (cardinality <= 0x00000002)      { return 1; }
    else if (cardinality <= 0x00000004) { return 2; }
    else if (cardinality <= 0x0000000F) { return 4; }
    else if (cardinality <= 0x000000FF) { return 8; }
    else if (cardinality <= 0x0000FFFF) { return 16; }
    else                                { return 32; }
}

static void
S_write_ord(void *ords, int32_t width, int32_t doc_id, int32_t ord) {
    switch (width) {
        case 1:
            if (ord) { NumUtil_u1set(ords, doc_id); }
            else     { NumUtil_u1clear(ords, doc_id); }
            break;
        case 2:
            NumUtil_u2set(ords, doc_id, ord);
            break;
        case 4:
            NumUtil_u4set(ords, doc_id, ord);
            break;
        case 8: {
                uint8_t *ints = (uint8_t*)ords;
                ints[doc_id] = ord;
            }
            break;
        case 16: {
                uint8_t *bytes = (uint8_t*)ords;
                bytes += doc_id * sizeof(uint16_t);
                NumUtil_encode_bigend_u16(ord, &bytes);
            }
            break;
        case 32: {
                uint8_t *bytes = (uint8_t*)ords;
                bytes += doc_id * sizeof(uint32_t);
                NumUtil_encode_bigend_u32(ord, &bytes);
            }
            break;
        default:
            THROW(ERR, "Invalid width: %i32", width);
    }
}

static void
S_write_val(Obj *val, int8_t prim_id, OutStream *ix_out, OutStream *dat_out,
            int64_t dat_start) {
    if (val) {
        switch (prim_id & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT: {
                    CharBuf *string = (CharBuf*)val;
                    int64_t dat_pos = OutStream_Tell(dat_out) - dat_start;
                    OutStream_Write_I64(ix_out, dat_pos);
                    OutStream_Write_Bytes(dat_out, (char*)CB_Get_Ptr8(string),
                                          CB_Get_Size(string));
                    break;
                }
            case FType_BLOB: {
                    ByteBuf *byte_buf = (ByteBuf*)val;
                    int64_t dat_pos = OutStream_Tell(dat_out) - dat_start;
                    OutStream_Write_I64(ix_out, dat_pos);
                    OutStream_Write_Bytes(dat_out, BB_Get_Buf(byte_buf),
                                          BB_Get_Size(byte_buf));
                    break;
                }
            case FType_INT32: {
                    Integer32 *i32 = (Integer32*)val;
                    OutStream_Write_I32(dat_out, Int32_Get_Value(i32));
                    break;
                }
            case FType_INT64: {
                    Integer64 *i64 = (Integer64*)val;
                    OutStream_Write_I64(dat_out, Int64_Get_Value(i64));
                    break;
                }
            case FType_FLOAT64: {
                    Float64 *float64 = (Float64*)val;
                    OutStream_Write_F64(dat_out, Float64_Get_Value(float64));
                    break;
                }
            case FType_FLOAT32: {
                    Float32 *float32 = (Float32*)val;
                    OutStream_Write_F32(dat_out, Float32_Get_Value(float32));
                    break;
                }
            default:
                THROW(ERR, "Unrecognized primitive id: %i32", (int32_t)prim_id);
        }
    }
    else {
        switch (prim_id & FType_PRIMITIVE_ID_MASK) {
            case FType_TEXT:
            case FType_BLOB: {
                    int64_t dat_pos = OutStream_Tell(dat_out) - dat_start;
                    OutStream_Write_I64(ix_out, dat_pos);
                }
                break;
            case FType_INT32:
                OutStream_Write_I32(dat_out, 0);
                break;
            case FType_INT64:
                OutStream_Write_I64(dat_out, 0);
                break;
            case FType_FLOAT64:
                OutStream_Write_F64(dat_out, 0.0);
                break;
            case FType_FLOAT32:
                OutStream_Write_F32(dat_out, 0.0f);
                break;
            default:
                THROW(ERR, "Unrecognized primitive id: %i32", (int32_t)prim_id);
        }
    }
}

int
SortFieldWriter_compare(SortFieldWriter *self, void *va, void *vb) {
    SFWriterElem *a = (SFWriterElem*)va;
    SFWriterElem *b = (SFWriterElem*)vb;
    int32_t comparison
        = FType_null_back_compare_values(self->type, a->value, b->value);
    if (comparison == 0) { comparison = b->doc_id - a->doc_id; }
    return comparison;
}

static int
S_compare_doc_ids_by_ord_rev(void *context, const void *va, const void *vb) {
    SortCache *sort_cache = (SortCache*)context;
    int32_t a = *(int32_t*)va;
    int32_t b = *(int32_t*)vb;
    int32_t ord_a = SortCache_Ordinal(sort_cache, a);
    int32_t ord_b = SortCache_Ordinal(sort_cache, b);
    return ord_a - ord_b;
}

static void
S_lazy_init_sorted_ids(SortFieldWriter *self) {
    if (!self->sorted_ids) {
        self->sorted_ids
            = (int32_t*)MALLOCATE((self->run_max + 1) * sizeof(int32_t));
        for (int32_t i = 0, max = self->run_max; i <= max; i++) {
            self->sorted_ids[i] = i;
        }
        Sort_quicksort(self->sorted_ids + 1, self->run_max, sizeof(int32_t),
                       S_compare_doc_ids_by_ord_rev, self->sort_cache);
    }
}

void
SortFieldWriter_flush(SortFieldWriter *self) {
    // Don't add a run unless we have data to put in it.
    if (SortFieldWriter_Cache_Count(self) == 0) { return; }

    OutStream *const temp_ord_out = self->temp_ord_out;
    OutStream *const temp_ix_out  = self->temp_ix_out;
    OutStream *const temp_dat_out = self->temp_dat_out;

    SortFieldWriter_Sort_Cache(self);
    SortFieldWriter *run
        = SortFieldWriter_new(self->schema, self->snapshot, self->segment,
                              self->polyreader, self->field, self->mem_pool,
                              self->mem_thresh, NULL, NULL, NULL);

    // Record stream starts and align.
    run->ord_start = OutStream_Align(temp_ord_out, sizeof(int64_t));
    if (self->var_width) {
        run->ix_start  = OutStream_Align(temp_ix_out, sizeof(int64_t));
    }
    run->dat_start = OutStream_Align(temp_dat_out, sizeof(int64_t));

    // Have the run borrow the array of elems.
    run->cache      = self->cache;
    run->cache_max  = self->cache_max;
    run->cache_tick = self->cache_tick;
    run->cache_cap  = self->cache_cap;

    // Write files, record stats.
    run->run_max = (int32_t)Seg_Get_Count(self->segment);
    run->run_cardinality = S_write_files(run, temp_ord_out, temp_ix_out,
                                         temp_dat_out);

    // Reclaim the buffer from the run and empty it.
    run->cache       = NULL;
    run->cache_max   = 0;
    run->cache_tick  = 0;
    run->cache_cap   = 0;
    self->cache_tick = self->cache_max;
    SortFieldWriter_Clear_Cache(self);

    // Record stream ends.
    run->ord_end = OutStream_Tell(temp_ord_out);
    if (self->var_width) {
        run->ix_end  = OutStream_Tell(temp_ix_out);
    }
    run->dat_end = OutStream_Tell(temp_dat_out);

    // Add the run to the array.
    SortFieldWriter_Add_Run(self, (SortExternal*)run);
}

uint32_t
SortFieldWriter_refill(SortFieldWriter *self) {
    if (!self->sort_cache) { return 0; }

    // Sanity check, then reset the cache and prepare to start loading items.
    uint32_t cache_count = SortFieldWriter_Cache_Count(self);
    if (cache_count) {
        THROW(ERR, "Refill called but cache contains %u32 items",
              cache_count);
    }
    SortFieldWriter_Clear_Cache(self);
    MemPool_Release_All(self->mem_pool);
    S_lazy_init_sorted_ids(self);

    const int32_t    null_ord   = self->null_ord;
    Hash *const      uniq_vals  = self->uniq_vals;
    I32Array *const  doc_map    = self->doc_map;
    SortCache *const sort_cache = self->sort_cache;
    Obj *const       blank      = SortCache_Make_Blank(sort_cache);

    while (self->run_ord < self->run_cardinality
           && MemPool_Get_Consumed(self->mem_pool) < self->mem_thresh
          ) {
        Obj *val = SortCache_Value(sort_cache, self->run_ord, blank);
        if (val) {
            Hash_Store(uniq_vals, val, INCREF(&EMPTY));
            break;
        }
        self->run_ord++;
    }
    uint32_t count = 0;
    while (self->run_tick <= self->run_max) {
        int32_t raw_doc_id = self->sorted_ids[self->run_tick];
        int32_t ord = SortCache_Ordinal(sort_cache, raw_doc_id);
        if (ord != null_ord) {
            int32_t remapped = doc_map
                               ? I32Arr_Get(doc_map, raw_doc_id)
                               : raw_doc_id;
            if (remapped) {
                Obj *val = SortCache_Value(sort_cache, ord, blank);
                SortFieldWriter_Add(self, remapped, val);
                count++;
            }
        }
        else if (ord > self->run_ord) {
            break;
        }
        self->run_tick++;
    }
    self->run_ord++;
    SortFieldWriter_Sort_Cache(self);

    if (self->run_ord >= self->run_cardinality) {
        DECREF(self->sort_cache);
        self->sort_cache = NULL;
    }

    DECREF(blank);
    return count;
}

void
SortFieldWriter_flip(SortFieldWriter *self) {
    uint32_t num_items = SortFieldWriter_Cache_Count(self);
    uint32_t num_runs = VA_Get_Size(self->runs);

    if (self->flipped) { THROW(ERR, "Can't call Flip() twice"); }
    self->flipped = true;

    // Sanity check.
    if (num_runs && num_items) {
        THROW(ERR, "Sanity check failed: num_runs: %u32 num_items: %u32",
              num_runs, num_items);
    }

    if (num_items) {
        SortFieldWriter_Sort_Cache(self);
    }
    else if (num_runs) {
        Folder  *folder = PolyReader_Get_Folder(self->polyreader);
        CharBuf *seg_name = Seg_Get_Name(self->segment);
        CharBuf *filepath = CB_newf("%o/sort_ord_temp", seg_name);
        self->ord_in = Folder_Open_In(folder, filepath);
        if (!self->ord_in) { RETHROW(INCREF(Err_get_error())); }
        if (self->var_width) {
            CB_setf(filepath, "%o/sort_ix_temp", seg_name);
            self->ix_in = Folder_Open_In(folder, filepath);
            if (!self->ix_in) { RETHROW(INCREF(Err_get_error())); }
        }
        CB_setf(filepath, "%o/sort_dat_temp", seg_name);
        self->dat_in = Folder_Open_In(folder, filepath);
        if (!self->dat_in) { RETHROW(INCREF(Err_get_error())); }
        DECREF(filepath);

        // Assign streams and a slice of mem_thresh.
        size_t sub_thresh = self->mem_thresh / num_runs;
        if (sub_thresh < 65536) { sub_thresh = 65536; }
        for (uint32_t i = 0; i < num_runs; i++) {
            SortFieldWriter *run = (SortFieldWriter*)VA_Fetch(self->runs, i);
            S_flip_run(run, sub_thresh, self->ord_in, self->ix_in,
                       self->dat_in);
        }
    }

    self->flipped = true;
}

static int32_t
S_write_files(SortFieldWriter *self, OutStream *ord_out, OutStream *ix_out,
              OutStream *dat_out) {
    int8_t    prim_id   = self->prim_id;
    int32_t   doc_max   = (int32_t)Seg_Get_Count(self->segment);
    bool_t    has_nulls = self->count == doc_max ? false : true;
    size_t    size      = (doc_max + 1) * sizeof(int32_t);
    int32_t  *ords      = (int32_t*)MALLOCATE(size);
    int32_t   ord       = 0;
    int64_t   dat_start = OutStream_Tell(dat_out);

    // Assign -1 as a stand-in for the NULL ord.
    for (int32_t i = 0; i <= doc_max; i++) {
        ords[i] = -1;
    }

    // Grab the first item and record its ord.  Add a dummy ord for invalid
    // doc id 0.
    SFWriterElem *elem = (SFWriterElem*)SortFieldWriter_Fetch(self);
    ords[elem->doc_id] = ord;
    ords[0] = 0;

    // Build array of ords, write non-NULL sorted values.
    Obj *val = Obj_Clone(elem->value);
    Obj *last_val_address = elem->value;
    S_write_val(elem->value, prim_id, ix_out, dat_out, dat_start);
    while (NULL != (elem = (SFWriterElem*)SortFieldWriter_Fetch(self))) {
        if (elem->value != last_val_address) {
            int32_t comparison
                = FType_Compare_Values(self->type, elem->value, val);
            if (comparison != 0) {
                ord++;
                S_write_val(elem->value, prim_id, ix_out, dat_out, dat_start);
                Obj_Mimic(val, elem->value);
            }
            last_val_address = elem->value;
        }
        ords[elem->doc_id] = ord;
    }
    DECREF(val);

    // If there are NULL values, write one now and record the NULL ord.
    if (has_nulls) {
        S_write_val(NULL, prim_id, ix_out, dat_out, dat_start);
        ord++;
        self->null_ord = ord;
    }
    int32_t null_ord = self->null_ord;

    // Write one extra file pointer so that we can always derive length.
    if (self->var_width) {
        OutStream_Write_I64(ix_out, OutStream_Tell(dat_out) - dat_start);
    }

    // Calculate cardinality and ord width.
    int32_t cardinality = ord + 1;
    self->ord_width     = S_calc_width(cardinality);
    int32_t ord_width   = self->ord_width;

    // Write ords.
    const double BITS_PER_BYTE = 8.0;
    double bytes_per_doc = ord_width / BITS_PER_BYTE;
    double byte_count = ceil((doc_max + 1) * bytes_per_doc);
    char *compressed_ords
        = (char*)CALLOCATE((size_t)byte_count, sizeof(char));
    for (int32_t i = 0; i <= doc_max; i++) {
        int32_t real_ord = ords[i] == -1 ? null_ord : ords[i];
        S_write_ord(compressed_ords, ord_width, i, real_ord);
    }
    OutStream_Write_Bytes(ord_out, compressed_ords, (size_t)byte_count);
    FREEMEM(compressed_ords);

    FREEMEM(ords);
    return cardinality;
}

int32_t
SortFieldWriter_finish(SortFieldWriter *self) {
    // Bail if there's no data.
    if (!SortFieldWriter_Peek(self)) { return 0; }

    int32_t  field_num = self->field_num;
    Folder  *folder    = PolyReader_Get_Folder(self->polyreader);
    CharBuf *seg_name  = Seg_Get_Name(self->segment);
    CharBuf *path      = CB_newf("%o/sort-%i32.ord", seg_name, field_num);

    // Open streams.
    OutStream *ord_out = Folder_Open_Out(folder, path);
    if (!ord_out) { RETHROW(INCREF(Err_get_error())); }
    OutStream *ix_out = NULL;
    if (self->var_width) {
        CB_setf(path, "%o/sort-%i32.ix", seg_name, field_num);
        ix_out = Folder_Open_Out(folder, path);
        if (!ix_out) { RETHROW(INCREF(Err_get_error())); }
    }
    CB_setf(path, "%o/sort-%i32.dat", seg_name, field_num);
    OutStream *dat_out = Folder_Open_Out(folder, path);
    if (!dat_out) { RETHROW(INCREF(Err_get_error())); }
    DECREF(path);

    int32_t cardinality = S_write_files(self, ord_out, ix_out, dat_out);

    // Close streams.
    OutStream_Close(ord_out);
    if (ix_out) { OutStream_Close(ix_out); }
    OutStream_Close(dat_out);
    DECREF(dat_out);
    DECREF(ix_out);
    DECREF(ord_out);

    return cardinality;
}

static void
S_flip_run(SortFieldWriter *run, size_t sub_thresh, InStream *ord_in,
           InStream *ix_in, InStream *dat_in) {
    if (run->flipped) { THROW(ERR, "Can't Flip twice"); }
    run->flipped = true;

    // Get our own MemoryPool, ZombieKeyedHash, and slice of mem_thresh.
    DECREF(run->uniq_vals);
    DECREF(run->mem_pool);
    run->mem_pool   = MemPool_new(0);
    run->uniq_vals  = (Hash*)ZKHash_new(run->mem_pool, run->prim_id);
    run->mem_thresh = sub_thresh;

    // Done if we already have a SortCache to read from.
    if (run->sort_cache) { return; }

    // Open the temp files for reading.
    CharBuf *seg_name = Seg_Get_Name(run->segment);
    CharBuf *alias    = CB_newf("%o/sort_ord_temp-%i64-to-%i64", seg_name,
                                run->ord_start, run->ord_end);
    InStream *ord_in_dupe = InStream_Reopen(ord_in, alias, run->ord_start,
                                            run->ord_end - run->ord_start);
    InStream *ix_in_dupe = NULL;
    if (run->var_width) {
        CB_setf(alias, "%o/sort_ix_temp-%i64-to-%i64", seg_name,
                run->ix_start, run->ix_end);
        ix_in_dupe = InStream_Reopen(ix_in, alias, run->ix_start,
                                     run->ix_end - run->ix_start);
    }
    CB_setf(alias, "%o/sort_dat_temp-%i64-to-%i64", seg_name,
            run->dat_start, run->dat_end);
    InStream *dat_in_dupe = InStream_Reopen(dat_in, alias, run->dat_start,
                                            run->dat_end - run->dat_start);
    DECREF(alias);

    // Get a SortCache.
    CharBuf *field = Seg_Field_Name(run->segment, run->field_num);
    switch (run->prim_id & FType_PRIMITIVE_ID_MASK) {
        case FType_TEXT:
            run->sort_cache = (SortCache*)TextSortCache_new(
                                  field, run->type, run->run_cardinality,
                                  run->run_max, run->null_ord,
                                  run->ord_width, ord_in_dupe,
                                  ix_in_dupe, dat_in_dupe);
            break;
        case FType_INT32:
            run->sort_cache = (SortCache*)I32SortCache_new(
                                  field, run->type, run->run_cardinality,
                                  run->run_max, run->null_ord,
                                  run->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_INT64:
            run->sort_cache = (SortCache*)I64SortCache_new(
                                  field, run->type, run->run_cardinality,
                                  run->run_max, run->null_ord,
                                  run->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_FLOAT32:
            run->sort_cache = (SortCache*)F32SortCache_new(
                                  field, run->type, run->run_cardinality,
                                  run->run_max, run->null_ord,
                                  run->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_FLOAT64:
            run->sort_cache = (SortCache*)F64SortCache_new(
                                  field, run->type, run->run_cardinality,
                                  run->run_max, run->null_ord,
                                  run->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        default:
            THROW(ERR, "No SortCache class for %o", run->type);
    }

    DECREF(ord_in_dupe);
    DECREF(ix_in_dupe);
    DECREF(dat_in_dupe);
}


