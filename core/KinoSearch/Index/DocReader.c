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

#define C_KINO_DOCREADER
#define C_KINO_POLYDOCREADER
#define C_KINO_DEFAULTDOCREADER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/DocReader.h"
#include "KinoSearch/Index/DocWriter.h"
#include "KinoSearch/Index/PolyReader.h"
#include "KinoSearch/Index/Segment.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Store/InStream.h"

DocReader*
DocReader_init(DocReader *self, Schema *schema, Folder *folder, 
               Snapshot *snapshot, VArray *segments, int32_t seg_tick)
{
    return (DocReader*)DataReader_init((DataReader*)self, schema, folder, 
        snapshot, segments, seg_tick);
}

DocReader*
DocReader_aggregator(DocReader *self, VArray *readers, I32Array *offsets)
{
    UNUSED_VAR(self);
    return (DocReader*)PolyDocReader_new(readers, offsets);
}

PolyDocReader*
PolyDocReader_new(VArray *readers, I32Array *offsets)
{
    PolyDocReader *self = (PolyDocReader*)VTable_Make_Obj(POLYDOCREADER);
    return PolyDocReader_init(self, readers, offsets);
}

PolyDocReader*
PolyDocReader_init(PolyDocReader *self, VArray *readers, I32Array *offsets)
{
    uint32_t i, max;
    DocReader_init((DocReader*)self, NULL, NULL, NULL, NULL, -1);
    for (i = 0, max = VA_Get_Size(readers); i < max; i++) {
        CERTIFY(VA_Fetch(readers, i), DOCREADER);
    }
    self->readers = (VArray*)INCREF(readers);
    self->offsets = (I32Array*)INCREF(offsets);
    return self;
}

void
PolyDocReader_close(PolyDocReader *self)
{
    if (self->readers) {
        uint32_t i, max;
        for (i = 0, max = VA_Get_Size(self->readers); i < max; i++) {
            DocReader *reader = (DocReader*)VA_Fetch(self->readers, i);
            if (reader) { DocReader_Close(reader); }
        }
        VA_Clear(self->readers);
    }
}

void
PolyDocReader_destroy(PolyDocReader *self)
{
    DECREF(self->readers);
    DECREF(self->offsets);
    SUPER_DESTROY(self, POLYDOCREADER);
}

Obj*
PolyDocReader_fetch(PolyDocReader *self, int32_t doc_id, float score, 
                    int32_t offset)
{
    uint32_t seg_tick  = PolyReader_sub_tick(self->offsets, doc_id);
    int32_t my_offset = I32Arr_Get(self->offsets, seg_tick);
    DocReader *doc_reader = (DocReader*)VA_Fetch(self->readers, seg_tick);
    Obj *hit = NULL;
    if (!doc_reader) { 
        THROW(ERR, "Invalid doc_id: %i32", doc_id); 
    }
    else {
        hit = DocReader_Fetch(doc_reader, doc_id - my_offset, score, 
            offset + my_offset);
    }
    return hit;
}

DefaultDocReader*
DefDocReader_new(Schema *schema, Folder *folder, Snapshot *snapshot, 
                 VArray *segments, int32_t seg_tick)
{
    DefaultDocReader *self 
        = (DefaultDocReader*)VTable_Make_Obj(DEFAULTDOCREADER);
    return DefDocReader_init(self, schema, folder, snapshot, segments,
        seg_tick);
}

void
DefDocReader_close(DefaultDocReader *self)
{
    if (self->dat_in != NULL) {
        InStream_Close(self->dat_in);
        DECREF(self->dat_in);
        self->dat_in = NULL;
    }
    if (self->ix_in != NULL) {
        InStream_Close(self->ix_in);
        DECREF(self->ix_in);
        self->ix_in = NULL;
    }
}

void
DefDocReader_destroy(DefaultDocReader *self)
{
    DECREF(self->ix_in);
    DECREF(self->dat_in);
    SUPER_DESTROY(self, DEFAULTDOCREADER);
}

DefaultDocReader*
DefDocReader_init(DefaultDocReader *self, Schema *schema, Folder *folder, 
                  Snapshot *snapshot, VArray *segments, int32_t seg_tick)
{
    Hash *metadata; 
    Segment *segment;
    DocReader_init((DocReader*)self, schema, folder, snapshot, segments,
        seg_tick);
    segment = DefDocReader_Get_Segment(self);
    metadata = (Hash*)Seg_Fetch_Metadata_Str(segment, "documents", 9);

    if (metadata) {
        CharBuf *seg_name  = Seg_Get_Name(segment);
        CharBuf *ix_file   = CB_newf("%o/documents.ix", seg_name);
        CharBuf *dat_file  = CB_newf("%o/documents.dat", seg_name);
        Obj     *format    = Hash_Fetch_Str(metadata, "format", 6);

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
            self->ix_in = Folder_Open_In(folder, ix_file);
            if (!self->ix_in) {
                Err *error = (Err*)INCREF(Err_get_error());
                DECREF(ix_file);
                DECREF(dat_file);
                DECREF(self);
                RETHROW(error);
            }
            self->dat_in = Folder_Open_In(folder, dat_file);
            if (!self->dat_in) {
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
DefDocReader_read_record(DefaultDocReader *self, ByteBuf *buffer,
                         int32_t doc_id)
{
    int64_t  start;
    int64_t  end;
    size_t   size;
    char    *buf;

    // Find start and length of variable length record. 
    InStream_Seek(self->ix_in, (int64_t)doc_id * 8);
    start = InStream_Read_I64(self->ix_in);
    end   = InStream_Read_I64(self->ix_in);
    size  = (size_t)(end - start);

    // Read in the record. 
    buf = BB_Grow(buffer, size);
    InStream_Seek(self->dat_in, start);
    InStream_Read_Bytes(self->dat_in, buf, size);
    BB_Set_Size(buffer, size);
}


