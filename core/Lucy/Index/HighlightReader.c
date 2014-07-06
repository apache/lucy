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

#define C_LUCY_HIGHLIGHTREADER
#define C_LUCY_POLYHIGHLIGHTREADER
#define C_LUCY_DEFAULTHIGHLIGHTREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/HighlightReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/HighlightWriter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/Freezer.h"

HighlightReader*
HLReader_init(HighlightReader *self, Schema *schema, Folder *folder,
              Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    ABSTRACT_CLASS_CHECK(self, HIGHLIGHTREADER);
    return self;
}

HighlightReader*
HLReader_Aggregator_IMP(HighlightReader *self, VArray *readers,
                        I32Array *offsets) {
    UNUSED_VAR(self);
    return (HighlightReader*)PolyHLReader_new(readers, offsets);
}

PolyHighlightReader*
PolyHLReader_new(VArray *readers, I32Array *offsets) {
    PolyHighlightReader *self
        = (PolyHighlightReader*)Class_Make_Obj(POLYHIGHLIGHTREADER);
    return PolyHLReader_init(self, readers, offsets);
}

PolyHighlightReader*
PolyHLReader_init(PolyHighlightReader *self, VArray *readers,
                  I32Array *offsets) {
    HLReader_init((HighlightReader*)self, NULL, NULL, NULL, NULL, -1);
    PolyHighlightReaderIVARS *const ivars = PolyHLReader_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(readers); i < max; i++) {
        CERTIFY(VA_Fetch(readers, i), HIGHLIGHTREADER);
    }
    ivars->readers = (VArray*)INCREF(readers);
    ivars->offsets = (I32Array*)INCREF(offsets);
    return self;
}

void
PolyHLReader_Close_IMP(PolyHighlightReader *self) {
    PolyHighlightReaderIVARS *const ivars = PolyHLReader_IVARS(self);
    if (ivars->readers) {
        for (uint32_t i = 0, max = VA_Get_Size(ivars->readers); i < max; i++) {
            HighlightReader *sub_reader
                = (HighlightReader*)VA_Fetch(ivars->readers, i);
            if (sub_reader) { HLReader_Close(sub_reader); }
        }
        DECREF(ivars->readers);
        DECREF(ivars->offsets);
        ivars->readers = NULL;
        ivars->offsets = NULL;
    }
}

void
PolyHLReader_Destroy_IMP(PolyHighlightReader *self) {
    PolyHighlightReaderIVARS *const ivars = PolyHLReader_IVARS(self);
    DECREF(ivars->readers);
    DECREF(ivars->offsets);
    SUPER_DESTROY(self, POLYHIGHLIGHTREADER);
}

DocVector*
PolyHLReader_Fetch_Doc_Vec_IMP(PolyHighlightReader *self, int32_t doc_id) {
    PolyHighlightReaderIVARS *const ivars = PolyHLReader_IVARS(self);
    uint32_t seg_tick = PolyReader_sub_tick(ivars->offsets, doc_id);
    int32_t  offset   = I32Arr_Get(ivars->offsets, seg_tick);
    HighlightReader *sub_reader
        = (HighlightReader*)VA_Fetch(ivars->readers, seg_tick);
    if (!sub_reader) { THROW(ERR, "Invalid doc_id: %i32", doc_id); }
    return HLReader_Fetch_Doc_Vec(sub_reader, doc_id - offset);
}

DefaultHighlightReader*
DefHLReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                VArray *segments, int32_t seg_tick) {
    DefaultHighlightReader *self = (DefaultHighlightReader*)Class_Make_Obj(
                                       DEFAULTHIGHLIGHTREADER);
    return DefHLReader_init(self, schema, folder, snapshot, segments,
                            seg_tick);
}

DefaultHighlightReader*
DefHLReader_init(DefaultHighlightReader *self, Schema *schema,
                 Folder *folder, Snapshot *snapshot, VArray *segments,
                 int32_t seg_tick) {
    HLReader_init((HighlightReader*)self, schema, folder, snapshot,
                  segments, seg_tick);
    DefaultHighlightReaderIVARS *const ivars = DefHLReader_IVARS(self);
    Segment *segment    = DefHLReader_Get_Segment(self);
    Hash *metadata      = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "highlight", 9);
    if (!metadata) {
        metadata = (Hash*)Seg_Fetch_Metadata_Utf8(segment, "term_vectors", 12);
    }

    // Check format.
    if (metadata) {
        Obj *format = Hash_Fetch_Utf8(metadata, "format", 6);
        if (!format) { THROW(ERR, "Missing 'format' var"); }
        else {
            if (Obj_To_I64(format) != HLWriter_current_file_format) {
                THROW(ERR, "Unsupported highlight data format: %i64",
                      Obj_To_I64(format));
            }
        }
    }

    // Open instreams.
    String *seg_name = Seg_Get_Name(segment);
    String *ix_file  = Str_newf("%o/highlight.ix", seg_name);
    String *dat_file = Str_newf("%o/highlight.dat", seg_name);
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

    return self;
}

void
DefHLReader_Close_IMP(DefaultHighlightReader *self) {
    DefaultHighlightReaderIVARS *const ivars = DefHLReader_IVARS(self);
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
DefHLReader_Destroy_IMP(DefaultHighlightReader *self) {
    DefaultHighlightReaderIVARS *const ivars = DefHLReader_IVARS(self);
    DECREF(ivars->ix_in);
    DECREF(ivars->dat_in);
    SUPER_DESTROY(self, DEFAULTHIGHLIGHTREADER);
}

DocVector*
DefHLReader_Fetch_Doc_Vec_IMP(DefaultHighlightReader *self, int32_t doc_id) {
    DefaultHighlightReaderIVARS *const ivars = DefHLReader_IVARS(self);
    InStream *const ix_in  = ivars->ix_in;
    InStream *const dat_in = ivars->dat_in;
    DocVector *doc_vec = DocVec_new();

    InStream_Seek(ix_in, doc_id * 8);
    int64_t file_pos = InStream_Read_I64(ix_in);
    InStream_Seek(dat_in, file_pos);

    uint32_t num_fields = InStream_Read_C32(dat_in);
    while (num_fields--) {
        String *field = Freezer_read_string(dat_in);
        ByteBuf *field_buf = Freezer_read_bytebuf(dat_in);
        DocVec_Add_Field_Buf(doc_vec, field, field_buf);
        DECREF(field_buf);
        DECREF(field);
    }

    return doc_vec;
}

void
DefHLReader_Read_Record_IMP(DefaultHighlightReader *self, int32_t doc_id,
                            ByteBuf *target) {
    DefaultHighlightReaderIVARS *const ivars = DefHLReader_IVARS(self);
    InStream *dat_in = ivars->dat_in;
    InStream *ix_in  = ivars->ix_in;

    InStream_Seek(ix_in, doc_id * 8);

    // Copy the whole record.
    int64_t  filepos = InStream_Read_I64(ix_in);
    int64_t  end     = InStream_Read_I64(ix_in);
    size_t   size    = (size_t)(end - filepos);
    char    *buf     = BB_Grow(target, size);
    InStream_Seek(dat_in, filepos);
    InStream_Read_Bytes(dat_in, buf, size);
    BB_Set_Size(target, size);
}


