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

#define C_LUCY_DOCREADER
#define C_LUCY_POLYDOCREADER
#define C_LUCY_DEFAULTDOCREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/DocReader.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DocWriter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"

DocReader*
DocReader_init(DocReader *self, Schema *schema, Folder *folder,
               Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    return (DocReader*)DataReader_init((DataReader*)self, schema, folder,
                                       snapshot, segments, seg_tick);
}

DocReader*
DocReader_Aggregator_IMP(DocReader *self, VArray *readers,
                         I32Array *offsets) {
    UNUSED_VAR(self);
    return (DocReader*)PolyDocReader_new(readers, offsets);
}

PolyDocReader*
PolyDocReader_new(VArray *readers, I32Array *offsets) {
    PolyDocReader *self = (PolyDocReader*)Class_Make_Obj(POLYDOCREADER);
    return PolyDocReader_init(self, readers, offsets);
}

PolyDocReader*
PolyDocReader_init(PolyDocReader *self, VArray *readers, I32Array *offsets) {
    DocReader_init((DocReader*)self, NULL, NULL, NULL, NULL, -1);
    PolyDocReaderIVARS *const ivars = PolyDocReader_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(readers); i < max; i++) {
        CERTIFY(VA_Fetch(readers, i), DOCREADER);
    }
    ivars->readers = (VArray*)INCREF(readers);
    ivars->offsets = (I32Array*)INCREF(offsets);
    return self;
}

void
PolyDocReader_Close_IMP(PolyDocReader *self) {
    PolyDocReaderIVARS *const ivars = PolyDocReader_IVARS(self);
    if (ivars->readers) {
        for (uint32_t i = 0, max = VA_Get_Size(ivars->readers); i < max; i++) {
            DocReader *reader = (DocReader*)VA_Fetch(ivars->readers, i);
            if (reader) { DocReader_Close(reader); }
        }
        VA_Clear(ivars->readers);
    }
}

void
PolyDocReader_Destroy_IMP(PolyDocReader *self) {
    PolyDocReaderIVARS *const ivars = PolyDocReader_IVARS(self);
    DECREF(ivars->readers);
    DECREF(ivars->offsets);
    SUPER_DESTROY(self, POLYDOCREADER);
}

HitDoc*
PolyDocReader_Fetch_Doc_IMP(PolyDocReader *self, int32_t doc_id) {
    PolyDocReaderIVARS *const ivars = PolyDocReader_IVARS(self);
    uint32_t seg_tick = PolyReader_sub_tick(ivars->offsets, doc_id);
    int32_t  offset   = I32Arr_Get(ivars->offsets, seg_tick);
    DocReader *doc_reader = (DocReader*)VA_Fetch(ivars->readers, seg_tick);
    HitDoc *hit_doc = NULL;
    if (!doc_reader) {
        THROW(ERR, "Invalid doc_id: %i32", doc_id);
    }
    else {
        hit_doc = DocReader_Fetch_Doc(doc_reader, doc_id - offset);
        HitDoc_Set_Doc_ID(hit_doc, doc_id);
    }
    return hit_doc;
}

DefaultDocReader*
DefDocReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                 VArray *segments, int32_t seg_tick) {
    DefaultDocReader *self
        = (DefaultDocReader*)Class_Make_Obj(DEFAULTDOCREADER);
    return DefDocReader_init(self, schema, folder, snapshot, segments,
                             seg_tick);
}

void
DefDocReader_Close_IMP(DefaultDocReader *self) {
    DefaultDocReaderIVARS *const ivars = DefDocReader_IVARS(self);
    if (ivars->dat_in != NULL) {
        InStream_Close(ivars->dat_in);
        DECREF(ivars->dat_in);
        ivars->dat_in = NULL;
    }
    if (ivars->ix_in != NULL) {
        InStream_Close(ivars->ix_in);
        DECREF(ivars->ix_in);
        ivars->ix_in = NULL;
    }
}

void
DefDocReader_Destroy_IMP(DefaultDocReader *self) {
    DefaultDocReaderIVARS *const ivars = DefDocReader_IVARS(self);
    DECREF(ivars->ix_in);
    DECREF(ivars->dat_in);
    SUPER_DESTROY(self, DEFAULTDOCREADER);
}

DefaultDocReader*
DefDocReader_init(DefaultDocReader *self, Schema *schema, Folder *folder,
                  Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    Hash *metadata;
    Segment *segment;
    DocReader_init((DocReader*)self, schema, folder, snapshot, segments,
                   seg_tick);
    DefaultDocReaderIVARS *const ivars = DefDocReader_IVARS(self);
    segment = DefDocReader_Get_Segment(self);
    metadata = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "documents", 9);

    if (metadata) {
        String *seg_name  = Seg_Get_Name(segment);
        String *ix_file   = Str_newf("%o/documents.ix", seg_name);
        String *dat_file  = Str_newf("%o/documents.dat", seg_name);
        Obj     *format   = Hash_Fetch_Utf8(metadata, "format", 6);

        // Check format.
        if (!format) { THROW(ERR, "Missing 'format' var"); }
        else {
            int64_t format_val = Obj_To_I64(format);
            if (format_val < DocWriter_current_file_format) {
                THROW(ERR, "Obsolete doc storage format %i64; "
                      "Index regeneration is required", format_val);
            }
            else if (format_val != DocWriter_current_file_format) {
                THROW(ERR, "Unsupported doc storage format: %i64", format_val);
            }
        }

        // Get streams.
        if (Folder_Exists(folder, ix_file)) {
            ivars->ix_in = Folder_Open_In(folder, ix_file);
            if (!ivars->ix_in) {
                Err *error = (Err*)INCREF(Err_get_error());
                DECREF(ix_file);
                DECREF(dat_file);
                DECREF(self);
                RETHROW(error);
            }
            ivars->dat_in = Folder_Open_In(folder, dat_file);
            if (!ivars->dat_in) {
                Err *error = (Err*)INCREF(Err_get_error());
                DECREF(ix_file);
                DECREF(dat_file);
                DECREF(self);
                RETHROW(error);
            }
        }
        DECREF(ix_file);
        DECREF(dat_file);
    }

    return self;
}

void
DefDocReader_Read_Record_IMP(DefaultDocReader *self, ByteBuf *buffer,
                             int32_t doc_id) {
    DefaultDocReaderIVARS *const ivars = DefDocReader_IVARS(self);

    // Find start and length of variable length record.
    InStream_Seek(ivars->ix_in, (int64_t)doc_id * 8);
    int64_t start = InStream_Read_I64(ivars->ix_in);
    int64_t end   = InStream_Read_I64(ivars->ix_in);
    size_t size  = (size_t)(end - start);

    // Read in the record.
    char *buf = BB_Grow(buffer, size);
    InStream_Seek(ivars->dat_in, start);
    InStream_Read_Bytes(ivars->dat_in, buf, size);
    BB_Set_Size(buffer, size);
}


