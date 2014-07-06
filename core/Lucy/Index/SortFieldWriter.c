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
#define C_LUCY_SFWRITERELEM
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
#include "Lucy/Index/SortWriter.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/SortUtils.h"

// Prepare to read back a run.
static void
S_flip_run(SortFieldWriter *run, size_t sub_thresh, InStream *ord_in,
           InStream *ix_in, InStream *dat_in);

// Write out a sort cache.  Returns the number of unique values in the sort
// cache.
static int32_t
S_write_files(SortFieldWriter *self, OutStream *ord_out, OutStream *ix_out,
              OutStream *dat_out);

// Constructor for an element in the sort pool.
static SFWriterElem*
S_SFWriterElem_create(Obj *value, int32_t doc_id);

static int64_t
SI_increase_to_word_multiple(int64_t amount) {
    const int64_t remainder = amount % sizeof(void*);
    if (remainder) {
        amount += sizeof(void*);
        amount -= remainder;
    }
    return amount;
}

SortFieldWriter*
SortFieldWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
                    PolyReader *polyreader, String *field,
                    Counter *counter, size_t mem_thresh,
                    OutStream *temp_ord_out, OutStream *temp_ix_out,
                    OutStream *temp_dat_out) {
    SortFieldWriter *self
        = (SortFieldWriter*)Class_Make_Obj(SORTFIELDWRITER);
    return SortFieldWriter_init(self, schema, snapshot, segment, polyreader,
                                field, counter, mem_thresh, temp_ord_out,
                                temp_ix_out, temp_dat_out);
}

SortFieldWriter*
SortFieldWriter_init(SortFieldWriter *self, Schema *schema,
                     Snapshot *snapshot, Segment *segment,
                     PolyReader *polyreader, String *field,
                     Counter *counter, size_t mem_thresh,
                     OutStream *temp_ord_out, OutStream *temp_ix_out,
                     OutStream *temp_dat_out) {
    // Init.
    SortEx_init((SortExternal*)self);
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    ivars->null_ord        = -1;
    ivars->count           = 0;
    ivars->ord_start       = 0;
    ivars->ord_end         = 0;
    ivars->ix_start        = 0;
    ivars->ix_end          = 0;
    ivars->dat_start       = 0;
    ivars->dat_end         = 0;
    ivars->run_cardinality = -1;
    ivars->run_max         = -1;
    ivars->sort_cache      = NULL;
    ivars->doc_map         = NULL;
    ivars->sorted_ids      = NULL;
    ivars->run_tick        = 1;
    ivars->ord_width       = 0;

    // Assign.
    ivars->field        = Str_Clone(field);
    ivars->schema       = (Schema*)INCREF(schema);
    ivars->snapshot     = (Snapshot*)INCREF(snapshot);
    ivars->segment      = (Segment*)INCREF(segment);
    ivars->polyreader   = (PolyReader*)INCREF(polyreader);
    ivars->counter      = (Counter*)INCREF(counter);
    ivars->temp_ord_out = (OutStream*)INCREF(temp_ord_out);
    ivars->temp_ix_out  = (OutStream*)INCREF(temp_ix_out);
    ivars->temp_dat_out = (OutStream*)INCREF(temp_dat_out);
    ivars->mem_thresh   = mem_thresh;

    // Derive.
    ivars->field_num = Seg_Field_Num(segment, field);
    FieldType *type = (FieldType*)CERTIFY(
                          Schema_Fetch_Type(ivars->schema, field), FIELDTYPE);
    ivars->type    = (FieldType*)INCREF(type);
    ivars->prim_id = FType_Primitive_ID(type);
    ivars->mem_per_entry = Class_Get_Obj_Alloc_Size(SFWRITERELEM);
    if (ivars->prim_id == FType_TEXT) {
        ivars->mem_per_entry += Class_Get_Obj_Alloc_Size(STRING);
        ivars->var_width = true;
    }
    else if (ivars->prim_id == FType_BLOB) {
        ivars->mem_per_entry += Class_Get_Obj_Alloc_Size(BYTEBUF);
        ivars->var_width = true;
    }
    else {
        ivars->mem_per_entry += Class_Get_Obj_Alloc_Size(FLOAT64);
        ivars->var_width = false;
    }

    return self;
}

void
SortFieldWriter_Destroy_IMP(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    DECREF(ivars->field);
    DECREF(ivars->schema);
    DECREF(ivars->snapshot);
    DECREF(ivars->segment);
    DECREF(ivars->polyreader);
    DECREF(ivars->type);
    DECREF(ivars->counter);
    DECREF(ivars->temp_ord_out);
    DECREF(ivars->temp_ix_out);
    DECREF(ivars->temp_dat_out);
    DECREF(ivars->ord_in);
    DECREF(ivars->ix_in);
    DECREF(ivars->dat_in);
    DECREF(ivars->sort_cache);
    DECREF(ivars->doc_map);
    FREEMEM(ivars->sorted_ids);
    SUPER_DESTROY(self, SORTFIELDWRITER);
}

int32_t
SortFieldWriter_Get_Null_Ord_IMP(SortFieldWriter *self) {
    return SortFieldWriter_IVARS(self)->null_ord;
}

int32_t
SortFieldWriter_Get_Ord_Width_IMP(SortFieldWriter *self) {
    return SortFieldWriter_IVARS(self)->ord_width;
}

void
SortFieldWriter_Add_IMP(SortFieldWriter *self, int32_t doc_id, Obj *value) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    Counter *counter   = ivars->counter;
    Counter_Add(counter, ivars->mem_per_entry);
    if (ivars->prim_id == FType_TEXT) {
        int64_t size = Str_Get_Size((String*)value) + 1;
        size = SI_increase_to_word_multiple(size);
        Counter_Add(counter, size);
    }
    else if (ivars->prim_id == FType_BLOB) {
        int64_t size = BB_Get_Size((ByteBuf*)value) + 1;
        size = SI_increase_to_word_multiple(size);
        Counter_Add(counter, size);
    }
    SFWriterElem *elem = S_SFWriterElem_create(Obj_Clone(value), doc_id);
    SortFieldWriter_Feed(self, (Obj*)elem);
    ivars->count++;
}

void
SortFieldWriter_Add_Segment_IMP(SortFieldWriter *self, SegReader *reader,
                                I32Array *doc_map, SortCache *sort_cache) {
    if (!sort_cache) { return; }
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    SortFieldWriter *run
        = SortFieldWriter_new(ivars->schema, ivars->snapshot, ivars->segment,
                              ivars->polyreader, ivars->field, ivars->counter,
                              ivars->mem_thresh, NULL, NULL, NULL);
    SortFieldWriterIVARS *const run_ivars = SortFieldWriter_IVARS(run);
    run_ivars->sort_cache = (SortCache*)INCREF(sort_cache);
    run_ivars->doc_map    = (I32Array*)INCREF(doc_map);
    run_ivars->run_max    = SegReader_Doc_Max(reader);
    run_ivars->run_cardinality = SortCache_Get_Cardinality(sort_cache);
    run_ivars->null_ord   = SortCache_Get_Null_Ord(sort_cache);
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
                    String *string = (String*)val;
                    int64_t dat_pos = OutStream_Tell(dat_out) - dat_start;
                    OutStream_Write_I64(ix_out, dat_pos);
                    OutStream_Write_Bytes(dat_out, Str_Get_Ptr8(string),
                                          Str_Get_Size(string));
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
SortFieldWriter_Compare_IMP(SortFieldWriter *self, void *va, void *vb) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    SFWriterElemIVARS *a = SFWriterElem_IVARS(*(SFWriterElem**)va);
    SFWriterElemIVARS *b = SFWriterElem_IVARS(*(SFWriterElem**)vb);
    int32_t comparison
        = FType_null_back_compare_values(ivars->type, a->value, b->value);
    if (comparison == 0) { comparison = a->doc_id - b->doc_id; }
    return comparison;
}

static void
S_lazy_init_sorted_ids(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    if (ivars->sorted_ids) { return; }

    // Counting sort. Could be optimized by working directly on the
    // ordinal arrays.

    SortCache *sort_cache      = ivars->sort_cache;
    int32_t    run_cardinality = ivars->run_cardinality;
    int32_t    run_max         = ivars->run_max;

    // Count.
    int32_t *counts = (int32_t*)CALLOCATE(run_cardinality, sizeof(int32_t));
    for (int32_t doc_id = 0; doc_id <= run_max; ++doc_id) {
        int32_t ord = SortCache_Ordinal(sort_cache, doc_id);
        ++counts[ord];
    }

    // Compute partial sums.
    int32_t sum = 0;
    for (int32_t ord = 0; ord < run_cardinality; ++ord) {
        int32_t count = counts[ord];
        counts[ord] = sum;
        sum += count;
    }

    // Distribute.
    int32_t *sorted_ids = (int32_t*)MALLOCATE((run_max + 1) * sizeof(int32_t));
    for (int32_t doc_id = 0; doc_id <= run_max; ++doc_id) {
        int32_t ord = SortCache_Ordinal(sort_cache, doc_id);
        int32_t pos = counts[ord]++;
        sorted_ids[pos] = doc_id;
    }

    ivars->sorted_ids = sorted_ids;
    FREEMEM(counts);
}

void
SortFieldWriter_Flush_IMP(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);

    // Don't add a run unless we have data to put in it.
    if (SortFieldWriter_Buffer_Count(self) == 0) { return; }

    OutStream *const temp_ord_out = ivars->temp_ord_out;
    OutStream *const temp_ix_out  = ivars->temp_ix_out;
    OutStream *const temp_dat_out = ivars->temp_dat_out;

    SortFieldWriter_Sort_Buffer(self);
    SortFieldWriter *run
        = SortFieldWriter_new(ivars->schema, ivars->snapshot, ivars->segment,
                              ivars->polyreader, ivars->field, ivars->counter,
                              ivars->mem_thresh, NULL, NULL, NULL);
    SortFieldWriterIVARS *const run_ivars = SortFieldWriter_IVARS(run);

    // Record stream starts and align.
    run_ivars->ord_start = OutStream_Align(temp_ord_out, sizeof(int64_t));
    if (ivars->var_width) {
        run_ivars->ix_start  = OutStream_Align(temp_ix_out, sizeof(int64_t));
    }
    run_ivars->dat_start = OutStream_Align(temp_dat_out, sizeof(int64_t));

    // Have the run borrow the array of elems.
    run_ivars->buffer   = ivars->buffer;
    run_ivars->buf_max  = ivars->buf_max;
    run_ivars->buf_tick = ivars->buf_tick;
    run_ivars->buf_cap  = ivars->buf_cap;

    // Write files, record stats.
    run_ivars->run_max = (int32_t)Seg_Get_Count(ivars->segment);
    run_ivars->run_cardinality = S_write_files(run, temp_ord_out, temp_ix_out,
                                               temp_dat_out);

    // Reclaim the buffer from the run and empty it.
    run_ivars->buffer    = NULL;
    run_ivars->buf_max   = 0;
    run_ivars->buf_tick  = 0;
    run_ivars->buf_cap   = 0;
    ivars->buf_tick = ivars->buf_max;
    SortFieldWriter_Clear_Buffer(self);

    // Record stream ends.
    run_ivars->ord_end = OutStream_Tell(temp_ord_out);
    if (ivars->var_width) {
        run_ivars->ix_end  = OutStream_Tell(temp_ix_out);
    }
    run_ivars->dat_end = OutStream_Tell(temp_dat_out);

    // Add the run to the array.
    SortFieldWriter_Add_Run(self, (SortExternal*)run);
}

uint32_t
SortFieldWriter_Refill_IMP(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    if (!ivars->sort_cache) { return 0; }

    // Sanity check, then reset the buffer and prepare to start loading items.
    uint32_t buf_count = SortFieldWriter_Buffer_Count(self);
    if (buf_count) {
        THROW(ERR, "Refill called but buffer contains %u32 items",
              buf_count);
    }
    SortFieldWriter_Clear_Buffer(self);
    Counter_Reset(ivars->counter);
    S_lazy_init_sorted_ids(self);

    const int32_t    null_ord   = ivars->null_ord;
    I32Array *const  doc_map    = ivars->doc_map;
    SortCache *const sort_cache = ivars->sort_cache;

    uint32_t count = 0;
    while (ivars->run_tick <= ivars->run_max
           && Counter_Get_Value(ivars->counter) < ivars->mem_thresh
          ) {
        int32_t raw_doc_id = ivars->sorted_ids[ivars->run_tick];
        int32_t ord = SortCache_Ordinal(sort_cache, raw_doc_id);
        if (ord != null_ord) {
            int32_t remapped = doc_map
                               ? I32Arr_Get(doc_map, raw_doc_id)
                               : raw_doc_id;
            if (remapped) {
                Obj *val = SortCache_Value(sort_cache, ord);
                SortFieldWriter_Add(self, remapped, val);
                count++;
                DECREF(val);
            }
        }
        ivars->run_tick++;
    }

    if (ivars->run_tick > ivars->run_max) {
        DECREF(ivars->sort_cache);
        ivars->sort_cache = NULL;
        FREEMEM(ivars->sorted_ids);
        ivars->sorted_ids = NULL;
    }

    return count;
}

void
SortFieldWriter_Flip_IMP(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    uint32_t num_items = SortFieldWriter_Buffer_Count(self);
    uint32_t num_runs = VA_Get_Size(ivars->runs);

    if (ivars->flipped) { THROW(ERR, "Can't call Flip() twice"); }
    ivars->flipped = true;

    // Sanity check.
    if (num_runs && num_items) {
        THROW(ERR, "Sanity check failed: num_runs: %u32 num_items: %u32",
              num_runs, num_items);
    }

    if (num_items) {
        SortFieldWriter_Sort_Buffer(self);
    }
    else if (num_runs) {
        Folder  *folder = PolyReader_Get_Folder(ivars->polyreader);
        String *seg_name = Seg_Get_Name(ivars->segment);
        String *ord_path = Str_newf("%o/sort_ord_temp", seg_name);
        ivars->ord_in = Folder_Open_In(folder, ord_path);
        DECREF(ord_path);
        if (!ivars->ord_in) { RETHROW(INCREF(Err_get_error())); }
        if (ivars->var_width) {
            String *ix_path = Str_newf("%o/sort_ix_temp", seg_name);
            ivars->ix_in = Folder_Open_In(folder, ix_path);
            DECREF(ix_path);
            if (!ivars->ix_in) { RETHROW(INCREF(Err_get_error())); }
        }
        String *dat_path = Str_newf("%o/sort_dat_temp", seg_name);
        ivars->dat_in = Folder_Open_In(folder, dat_path);
        DECREF(dat_path);
        if (!ivars->dat_in) { RETHROW(INCREF(Err_get_error())); }

        // Assign streams and a slice of mem_thresh.
        size_t sub_thresh = ivars->mem_thresh / num_runs;
        if (sub_thresh < 65536) { sub_thresh = 65536; }
        for (uint32_t i = 0; i < num_runs; i++) {
            SortFieldWriter *run = (SortFieldWriter*)VA_Fetch(ivars->runs, i);
            S_flip_run(run, sub_thresh, ivars->ord_in, ivars->ix_in,
                       ivars->dat_in);
        }
    }

    ivars->flipped = true;
}

static int32_t
S_write_files(SortFieldWriter *self, OutStream *ord_out, OutStream *ix_out,
              OutStream *dat_out) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);
    int8_t    prim_id   = ivars->prim_id;
    int32_t   doc_max   = (int32_t)Seg_Get_Count(ivars->segment);
    bool      has_nulls = ivars->count == doc_max ? false : true;
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
    SFWriterElemIVARS *elem_ivars = SFWriterElem_IVARS(elem);
    ords[elem_ivars->doc_id] = ord;
    ords[0] = 0;

    // Build array of ords, write non-NULL sorted values.
    Obj *last_val = INCREF(elem_ivars->value);
    S_write_val(elem_ivars->value, prim_id, ix_out, dat_out, dat_start);
    DECREF(elem);
    while (NULL != (elem = (SFWriterElem*)SortFieldWriter_Fetch(self))) {
        elem_ivars = SFWriterElem_IVARS(elem);
        if (elem_ivars->value != last_val) {
            int32_t comparison
                = FType_Compare_Values(ivars->type, elem_ivars->value,
                                       last_val);
            if (comparison != 0) {
                ord++;
                S_write_val(elem_ivars->value, prim_id, ix_out, dat_out,
                            dat_start);
            }
            DECREF(last_val);
            last_val = INCREF(elem_ivars->value);
        }
        ords[elem_ivars->doc_id] = ord;
        DECREF(elem);
    }
    DECREF(last_val);

    // If there are NULL values, write one now and record the NULL ord.
    if (has_nulls) {
        S_write_val(NULL, prim_id, ix_out, dat_out, dat_start);
        ord++;
        ivars->null_ord = ord;
    }
    int32_t null_ord = ivars->null_ord;

    // Write one extra file pointer so that we can always derive length.
    if (ivars->var_width) {
        OutStream_Write_I64(ix_out, OutStream_Tell(dat_out) - dat_start);
    }

    // Calculate cardinality and ord width.
    int32_t cardinality = ord + 1;
    ivars->ord_width     = S_calc_width(cardinality);
    int32_t ord_width   = ivars->ord_width;

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
SortFieldWriter_Finish_IMP(SortFieldWriter *self) {
    SortFieldWriterIVARS *const ivars = SortFieldWriter_IVARS(self);

    // Bail if there's no data.
    if (!SortFieldWriter_Peek(self)) { return 0; }

    int32_t  field_num = ivars->field_num;
    Folder  *folder    = PolyReader_Get_Folder(ivars->polyreader);
    String  *seg_name  = Seg_Get_Name(ivars->segment);

    // Open streams.
    String *ord_path = Str_newf("%o/sort-%i32.ord", seg_name, field_num);
    OutStream *ord_out = Folder_Open_Out(folder, ord_path);
    DECREF(ord_path);
    if (!ord_out) { RETHROW(INCREF(Err_get_error())); }
    OutStream *ix_out = NULL;
    if (ivars->var_width) {
        String *ix_path = Str_newf("%o/sort-%i32.ix", seg_name, field_num);
        ix_out = Folder_Open_Out(folder, ix_path);
        DECREF(ix_path);
        if (!ix_out) { RETHROW(INCREF(Err_get_error())); }
    }
    String *dat_path = Str_newf("%o/sort-%i32.dat", seg_name, field_num);
    OutStream *dat_out = Folder_Open_Out(folder, dat_path);
    DECREF(dat_path);
    if (!dat_out) { RETHROW(INCREF(Err_get_error())); }

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
    SortFieldWriterIVARS *const run_ivars = SortFieldWriter_IVARS(run);

    if (run_ivars->flipped) { THROW(ERR, "Can't Flip twice"); }
    run_ivars->flipped = true;

    // Get our own slice of mem_thresh.
    DECREF(run_ivars->counter);
    run_ivars->counter    = Counter_new();
    run_ivars->mem_thresh = sub_thresh;

    // Done if we already have a SortCache to read from.
    if (run_ivars->sort_cache) { return; }

    // Open the temp files for reading.
    String *seg_name  = Seg_Get_Name(run_ivars->segment);
    String *ord_alias = Str_newf("%o/sort_ord_temp-%i64-to-%i64", seg_name,
                                 run_ivars->ord_start, run_ivars->ord_end);
    InStream *ord_in_dupe
        = InStream_Reopen(ord_in, ord_alias, run_ivars->ord_start,
                          run_ivars->ord_end - run_ivars->ord_start);
    DECREF(ord_alias);
    InStream *ix_in_dupe = NULL;
    if (run_ivars->var_width) {
        String *ix_alias = Str_newf("%o/sort_ix_temp-%i64-to-%i64", seg_name,
                                    run_ivars->ix_start, run_ivars->ix_end);
        ix_in_dupe = InStream_Reopen(ix_in, ix_alias, run_ivars->ix_start,
                                     run_ivars->ix_end - run_ivars->ix_start);
        DECREF(ix_alias);
    }
    String *dat_alias = Str_newf("%o/sort_dat_temp-%i64-to-%i64", seg_name,
                                 run_ivars->dat_start, run_ivars->dat_end);
    InStream *dat_in_dupe
        = InStream_Reopen(dat_in, dat_alias, run_ivars->dat_start,
                          run_ivars->dat_end - run_ivars->dat_start);
    DECREF(dat_alias);

    // Get a SortCache.
    String *field = Seg_Field_Name(run_ivars->segment, run_ivars->field_num);
    switch (run_ivars->prim_id & FType_PRIMITIVE_ID_MASK) {
        case FType_TEXT:
            run_ivars->sort_cache = (SortCache*)TextSortCache_new(
                                  field, run_ivars->type, run_ivars->run_cardinality,
                                  run_ivars->run_max, run_ivars->null_ord,
                                  run_ivars->ord_width, ord_in_dupe,
                                  ix_in_dupe, dat_in_dupe);
            break;
        case FType_INT32:
            run_ivars->sort_cache = (SortCache*)I32SortCache_new(
                                  field, run_ivars->type, run_ivars->run_cardinality,
                                  run_ivars->run_max, run_ivars->null_ord,
                                  run_ivars->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_INT64:
            run_ivars->sort_cache = (SortCache*)I64SortCache_new(
                                  field, run_ivars->type, run_ivars->run_cardinality,
                                  run_ivars->run_max, run_ivars->null_ord,
                                  run_ivars->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_FLOAT32:
            run_ivars->sort_cache = (SortCache*)F32SortCache_new(
                                  field, run_ivars->type, run_ivars->run_cardinality,
                                  run_ivars->run_max, run_ivars->null_ord,
                                  run_ivars->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        case FType_FLOAT64:
            run_ivars->sort_cache = (SortCache*)F64SortCache_new(
                                  field, run_ivars->type, run_ivars->run_cardinality,
                                  run_ivars->run_max, run_ivars->null_ord,
                                  run_ivars->ord_width, ord_in_dupe,
                                  dat_in_dupe);
            break;
        default:
            THROW(ERR, "No SortCache class for %o", run_ivars->type);
    }

    DECREF(ord_in_dupe);
    DECREF(ix_in_dupe);
    DECREF(dat_in_dupe);
}

/***************************************************************************/

static SFWriterElem*
S_SFWriterElem_create(Obj *value, int32_t doc_id) {
    SFWriterElem *self = (SFWriterElem*)Class_Make_Obj(SFWRITERELEM);
    SFWriterElemIVARS *ivars = SFWriterElem_IVARS(self);
    ivars->value  = value;
    ivars->doc_id = doc_id;
    return self;
}

void
SFWriterElem_Destroy_IMP(SFWriterElem *self) {
    SFWriterElemIVARS *ivars = SFWriterElem_IVARS(self);
    DECREF(ivars->value);
    SUPER_DESTROY(self, SFWRITERELEM);
}

