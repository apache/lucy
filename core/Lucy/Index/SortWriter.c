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

#define C_LUCY_SORTWRITER
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Lucy/Index/SortWriter.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SortCache.h"
#include "Lucy/Index/SortFieldWriter.h"
#include "Lucy/Index/SortReader.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Clownfish/Util/Memory.h"
#include "Lucy/Util/MemoryPool.h"
#include "Clownfish/Util/SortUtils.h"

int32_t SortWriter_current_file_format = 3;

static size_t default_mem_thresh = 0x400000; // 4 MB

SortWriter*
SortWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
               PolyReader *polyreader) {
    SortWriter *self = (SortWriter*)VTable_Make_Obj(SORTWRITER);
    return SortWriter_init(self, schema, snapshot, segment, polyreader);
}

SortWriter*
SortWriter_init(SortWriter *self, Schema *schema, Snapshot *snapshot,
                Segment *segment, PolyReader *polyreader) {
    uint32_t field_max = Schema_Num_Fields(schema) + 1;
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);

    // Init.
    ivars->field_writers   = VA_new(field_max);
    ivars->counts          = Hash_new(0);
    ivars->null_ords       = Hash_new(0);
    ivars->ord_widths      = Hash_new(0);
    ivars->temp_ord_out    = NULL;
    ivars->temp_ix_out     = NULL;
    ivars->temp_dat_out    = NULL;
    ivars->mem_pool        = MemPool_new(0);
    ivars->mem_thresh      = default_mem_thresh;
    ivars->flush_at_finish = false;

    return self;
}

void
SortWriter_Destroy_IMP(SortWriter *self) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);
    DECREF(ivars->field_writers);
    DECREF(ivars->counts);
    DECREF(ivars->null_ords);
    DECREF(ivars->ord_widths);
    DECREF(ivars->temp_ord_out);
    DECREF(ivars->temp_ix_out);
    DECREF(ivars->temp_dat_out);
    DECREF(ivars->mem_pool);
    SUPER_DESTROY(self, SORTWRITER);
}

void
SortWriter_set_default_mem_thresh(size_t mem_thresh) {
    default_mem_thresh = mem_thresh;
}

static SortFieldWriter*
S_lazy_init_field_writer(SortWriter *self, int32_t field_num) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);

    SortFieldWriter *field_writer
        = (SortFieldWriter*)VA_Fetch(ivars->field_writers, field_num);
    if (!field_writer) {

        // Open temp files.
        if (!ivars->temp_ord_out) {
            Folder  *folder   = ivars->folder;
            CharBuf *seg_name = Seg_Get_Name(ivars->segment);
            CharBuf *path     = CB_newf("%o/sort_ord_temp", seg_name);
            ivars->temp_ord_out = Folder_Open_Out(folder, path);
            if (!ivars->temp_ord_out) {
                DECREF(path);
                RETHROW(INCREF(Err_get_error()));
            }
            CB_setf(path, "%o/sort_ix_temp", seg_name);
            ivars->temp_ix_out = Folder_Open_Out(folder, path);
            if (!ivars->temp_ix_out) {
                DECREF(path);
                RETHROW(INCREF(Err_get_error()));
            }
            CB_setf(path, "%o/sort_dat_temp", seg_name);
            ivars->temp_dat_out = Folder_Open_Out(folder, path);
            if (!ivars->temp_dat_out) {
                DECREF(path);
                RETHROW(INCREF(Err_get_error()));
            }
            DECREF(path);
        }

        CharBuf *field = Seg_Field_Name(ivars->segment, field_num);
        field_writer
            = SortFieldWriter_new(ivars->schema, ivars->snapshot, ivars->segment,
                                  ivars->polyreader, field, ivars->mem_pool,
                                  ivars->mem_thresh, ivars->temp_ord_out,
                                  ivars->temp_ix_out, ivars->temp_dat_out);
        VA_Store(ivars->field_writers, field_num, (Obj*)field_writer);
    }
    return field_writer;
}

void
SortWriter_Add_Inverted_Doc_IMP(SortWriter *self, Inverter *inverter,
                                int32_t doc_id) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);
    int32_t field_num;

    Inverter_Iterate(inverter);
    while (0 != (field_num = Inverter_Next(inverter))) {
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Sortable(type)) {
            SortFieldWriter *field_writer
                = S_lazy_init_field_writer(self, field_num);
            SortFieldWriter_Add(field_writer, doc_id,
                                Inverter_Get_Value(inverter));
        }
    }

    // If our SortFieldWriters have collectively passed the memory threshold,
    // flush all of them, then release all unique values with a single action.
    if (MemPool_Get_Consumed(ivars->mem_pool) > ivars->mem_thresh) {
        for (uint32_t i = 0; i < VA_Get_Size(ivars->field_writers); i++) {
            SortFieldWriter *const field_writer
                = (SortFieldWriter*)VA_Fetch(ivars->field_writers, i);
            if (field_writer) { SortFieldWriter_Flush(field_writer); }
        }
        MemPool_Release_All(ivars->mem_pool);
        ivars->flush_at_finish = true;
    }
}

void
SortWriter_Add_Segment_IMP(SortWriter *self, SegReader *reader,
                           I32Array *doc_map) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);
    VArray *fields = Schema_All_Fields(ivars->schema);

    // Proceed field-at-a-time, rather than doc-at-a-time.
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        CharBuf *field = (CharBuf*)VA_Fetch(fields, i);
        SortReader *sort_reader = (SortReader*)SegReader_Fetch(
                                      reader, VTable_Get_Name(SORTREADER));
        SortCache *cache = sort_reader
                           ? SortReader_Fetch_Sort_Cache(sort_reader, field)
                           : NULL;
        if (cache) {
            int32_t field_num = Seg_Field_Num(ivars->segment, field);
            SortFieldWriter *field_writer
                = S_lazy_init_field_writer(self, field_num);
            SortFieldWriter_Add_Segment(field_writer, reader, doc_map, cache);
            ivars->flush_at_finish = true;
        }
    }

    DECREF(fields);
}

void
SortWriter_Finish_IMP(SortWriter *self) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);
    VArray *const field_writers = ivars->field_writers;

    // If we have no data, bail out.
    if (!ivars->temp_ord_out) { return; }

    // If we've either flushed or added segments, flush everything so that any
    // one field can use the entire margin up to mem_thresh.
    if (ivars->flush_at_finish) {
        for (uint32_t i = 1, max = VA_Get_Size(field_writers); i < max; i++) {
            SortFieldWriter *field_writer
                = (SortFieldWriter*)VA_Fetch(field_writers, i);
            if (field_writer) {
                SortFieldWriter_Flush(field_writer);
            }
        }
    }

    // Close down temp streams.
    OutStream_Close(ivars->temp_ord_out);
    OutStream_Close(ivars->temp_ix_out);
    OutStream_Close(ivars->temp_dat_out);

    for (uint32_t i = 1, max = VA_Get_Size(field_writers); i < max; i++) {
        SortFieldWriter *field_writer
            = (SortFieldWriter*)VA_Delete(field_writers, i);
        if (field_writer) {
            CharBuf *field = Seg_Field_Name(ivars->segment, i);
            SortFieldWriter_Flip(field_writer);
            int32_t count = SortFieldWriter_Finish(field_writer);
            Hash_Store(ivars->counts, (Obj*)field,
                       (Obj*)CB_newf("%i32", count));
            int32_t null_ord = SortFieldWriter_Get_Null_Ord(field_writer);
            if (null_ord != -1) {
                Hash_Store(ivars->null_ords, (Obj*)field,
                           (Obj*)CB_newf("%i32", null_ord));
            }
            int32_t ord_width = SortFieldWriter_Get_Ord_Width(field_writer);
            Hash_Store(ivars->ord_widths, (Obj*)field,
                       (Obj*)CB_newf("%i32", ord_width));
        }

        DECREF(field_writer);
    }
    VA_Clear(field_writers);

    // Store metadata.
    Seg_Store_Metadata_Str(ivars->segment, "sort", 4,
                           (Obj*)SortWriter_Metadata(self));

    // Clean up.
    Folder  *folder   = ivars->folder;
    CharBuf *seg_name = Seg_Get_Name(ivars->segment);
    CharBuf *path     = CB_newf("%o/sort_ord_temp", seg_name);
    Folder_Delete(folder, path);
    CB_setf(path, "%o/sort_ix_temp", seg_name);
    Folder_Delete(folder, path);
    CB_setf(path, "%o/sort_dat_temp", seg_name);
    Folder_Delete(folder, path);
    DECREF(path);
}

Hash*
SortWriter_Metadata_IMP(SortWriter *self) {
    SortWriterIVARS *const ivars = SortWriter_IVARS(self);
    SortWriter_Metadata_t super_meta
        = (SortWriter_Metadata_t)SUPER_METHOD_PTR(SORTWRITER,
                                                  Lucy_SortWriter_Metadata);
    Hash *const metadata = super_meta(self);
    Hash_Store_Str(metadata, "counts", 6, INCREF(ivars->counts));
    Hash_Store_Str(metadata, "null_ords", 9, INCREF(ivars->null_ords));
    Hash_Store_Str(metadata, "ord_widths", 10, INCREF(ivars->ord_widths));
    return metadata;
}

int32_t
SortWriter_Format_IMP(SortWriter *self) {
    UNUSED_VAR(self);
    return SortWriter_current_file_format;
}


