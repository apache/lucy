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

#define C_LUCY_DOCWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/DocWriter.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

static OutStream*
S_lazy_init(DocWriter *self);

int32_t DocWriter_current_file_format = 2;

DocWriter*
DocWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
              PolyReader *polyreader) {
    DocWriter *self = (DocWriter*)VTable_Make_Obj(DOCWRITER);
    return DocWriter_init(self, schema, snapshot, segment, polyreader);
}

DocWriter*
DocWriter_init(DocWriter *self, Schema *schema, Snapshot *snapshot,
               Segment *segment, PolyReader *polyreader) {
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    return self;
}

void
DocWriter_destroy(DocWriter *self) {
    DECREF(self->dat_out);
    DECREF(self->ix_out);
    SUPER_DESTROY(self, DOCWRITER);
}

static OutStream*
S_lazy_init(DocWriter *self) {
    if (!self->dat_out) {
        Folder  *folder   = self->folder;
        CharBuf *seg_name = Seg_Get_Name(self->segment);

        // Get streams.
        CharBuf *ix_file = CB_newf("%o/documents.ix", seg_name);
        self->ix_out = Folder_Open_Out(folder, ix_file);
        DECREF(ix_file);
        if (!self->ix_out) { RETHROW(INCREF(Err_get_error())); }
        CharBuf *dat_file = CB_newf("%o/documents.dat", seg_name);
        self->dat_out = Folder_Open_Out(folder, dat_file);
        DECREF(dat_file);
        if (!self->dat_out) { RETHROW(INCREF(Err_get_error())); }

        // Go past non-doc #0.
        OutStream_Write_I64(self->ix_out, 0);
    }

    return self->dat_out;
}

void
DocWriter_add_inverted_doc(DocWriter *self, Inverter *inverter,
                           int32_t doc_id) {
    OutStream *dat_out    = S_lazy_init(self);
    OutStream *ix_out     = self->ix_out;
    uint32_t   num_stored = 0;
    int64_t    start      = OutStream_Tell(dat_out);
    int64_t    expected   = OutStream_Tell(ix_out) / 8;

    // Verify doc id.
    if (doc_id != expected) {
        THROW(ERR, "Expected doc id %i64 but got %i32", expected, doc_id);
    }

    // Write the number of stored fields.
    Inverter_Iterate(inverter);
    while (Inverter_Next(inverter)) {
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Stored(type)) { num_stored++; }
    }
    OutStream_Write_C32(dat_out, num_stored);

    Inverter_Iterate(inverter);
    while (Inverter_Next(inverter)) {
        // Only store fields marked as "stored".
        FieldType *type = Inverter_Get_Type(inverter);
        if (FType_Stored(type)) {
            CharBuf *field = Inverter_Get_Field_Name(inverter);
            Obj *value = Inverter_Get_Value(inverter);
            Freezer_serialize_charbuf(field, dat_out);
            switch (FType_Primitive_ID(type) & FType_PRIMITIVE_ID_MASK) {
                case FType_TEXT: {
                    uint8_t *buf  = CB_Get_Ptr8((CharBuf*)value);
                    size_t   size = CB_Get_Size((CharBuf*)value);
                    OutStream_Write_C32(dat_out, size);
                    OutStream_Write_Bytes(dat_out, buf, size);
                    break;
                }
                case FType_BLOB: {
                    char   *buf  = BB_Get_Buf((ByteBuf*)value);
                    size_t  size = BB_Get_Size((ByteBuf*)value);
                    OutStream_Write_C32(dat_out, size);
                    OutStream_Write_Bytes(dat_out, buf, size);
                    break;
                }
                case FType_INT32: {
                    int32_t val = Int32_Get_Value((Integer32*)value);
                    OutStream_Write_C32(dat_out, val);
                    break;
                }
                case FType_INT64: {
                    int64_t val = Int64_Get_Value((Integer64*)value);
                    OutStream_Write_C64(dat_out, val);
                    break;
                }
                case FType_FLOAT32: {
                    float val = Float32_Get_Value((Float32*)value);
                    OutStream_Write_F32(dat_out, val);
                    break;
                }
                case FType_FLOAT64: {
                    double val = Float64_Get_Value((Float64*)value);
                    OutStream_Write_F64(dat_out, val);
                    break;
                }
                default:
                    THROW(ERR, "Unrecognized type: %o", type);
            }
        }
    }

    // Write file pointer.
    OutStream_Write_I64(ix_out, start);
}

void
DocWriter_add_segment(DocWriter *self, SegReader *reader,
                      I32Array *doc_map) {
    int32_t doc_max = SegReader_Doc_Max(reader);

    if (doc_max == 0) {
        // Bail if the supplied segment is empty.
        return;
    }
    else {
        OutStream *const dat_out = S_lazy_init(self);
        OutStream *const ix_out  = self->ix_out;
        ByteBuf   *const buffer  = BB_new(0);
        DefaultDocReader *const doc_reader
            = (DefaultDocReader*)CERTIFY(
                  SegReader_Obtain(reader, VTable_Get_Name(DOCREADER)),
                  DEFAULTDOCREADER);

        for (int32_t i = 1, max = SegReader_Doc_Max(reader); i <= max; i++) {
            if (I32Arr_Get(doc_map, i)) {
                int64_t  start = OutStream_Tell(dat_out);

                // Copy record over.
                DefDocReader_Read_Record(doc_reader, buffer, i);
                char *buf   = BB_Get_Buf(buffer);
                size_t size = BB_Get_Size(buffer);
                OutStream_Write_Bytes(dat_out, buf, size);

                // Write file pointer.
                OutStream_Write_I64(ix_out, start);
            }
        }

        DECREF(buffer);
    }
}

void
DocWriter_finish(DocWriter *self) {
    if (self->dat_out) {
        // Write one final file pointer, so that we can derive the length of
        // the last record.
        int64_t end = OutStream_Tell(self->dat_out);
        OutStream_Write_I64(self->ix_out, end);

        // Close down output streams.
        OutStream_Close(self->dat_out);
        OutStream_Close(self->ix_out);
        Seg_Store_Metadata_Str(self->segment, "documents", 9,
                               (Obj*)DocWriter_Metadata(self));
    }
}

int32_t
DocWriter_format(DocWriter *self) {
    UNUSED_VAR(self);
    return DocWriter_current_file_format;
}


