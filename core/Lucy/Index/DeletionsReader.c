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

#define C_LUCY_DELETIONSREADER
#define C_LUCY_POLYDELETIONSREADER
#define C_LUCY_DEFAULTDELETIONSREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/BitVecDelDocs.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/BitVecMatcher.h"
#include "Lucy/Search/SeriesMatcher.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Util/IndexFileNames.h"

DeletionsReader*
DelReader_init(DeletionsReader *self, Schema *schema, Folder *folder,
               Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    ABSTRACT_CLASS_CHECK(self, DELETIONSREADER);
    return self;
}

DeletionsReader*
DelReader_Aggregator_IMP(DeletionsReader *self, VArray *readers,
                         I32Array *offsets) {
    UNUSED_VAR(self);
    return (DeletionsReader*)PolyDelReader_new(readers, offsets);
}

PolyDeletionsReader*
PolyDelReader_new(VArray *readers, I32Array *offsets) {
    PolyDeletionsReader *self
        = (PolyDeletionsReader*)Class_Make_Obj(POLYDELETIONSREADER);
    return PolyDelReader_init(self, readers, offsets);
}

PolyDeletionsReader*
PolyDelReader_init(PolyDeletionsReader *self, VArray *readers,
                   I32Array *offsets) {
    DelReader_init((DeletionsReader*)self, NULL, NULL, NULL, NULL, -1);
    PolyDeletionsReaderIVARS *const ivars = PolyDelReader_IVARS(self);
    ivars->del_count = 0;
    for (uint32_t i = 0, max = VA_Get_Size(readers); i < max; i++) {
        DeletionsReader *reader = (DeletionsReader*)CERTIFY(
                                      VA_Fetch(readers, i), DELETIONSREADER);
        ivars->del_count += DelReader_Del_Count(reader);
    }
    ivars->readers = (VArray*)INCREF(readers);
    ivars->offsets = (I32Array*)INCREF(offsets);
    return self;
}

void
PolyDelReader_Close_IMP(PolyDeletionsReader *self) {
    PolyDeletionsReaderIVARS *const ivars = PolyDelReader_IVARS(self);
    if (ivars->readers) {
        for (uint32_t i = 0, max = VA_Get_Size(ivars->readers); i < max; i++) {
            DeletionsReader *reader
                = (DeletionsReader*)VA_Fetch(ivars->readers, i);
            if (reader) { DelReader_Close(reader); }
        }
        VA_Clear(ivars->readers);
    }
}

void
PolyDelReader_Destroy_IMP(PolyDeletionsReader *self) {
    PolyDeletionsReaderIVARS *const ivars = PolyDelReader_IVARS(self);
    DECREF(ivars->readers);
    DECREF(ivars->offsets);
    SUPER_DESTROY(self, POLYDELETIONSREADER);
}

int32_t
PolyDelReader_Del_Count_IMP(PolyDeletionsReader *self) {
    return PolyDelReader_IVARS(self)->del_count;
}

Matcher*
PolyDelReader_Iterator_IMP(PolyDeletionsReader *self) {
    PolyDeletionsReaderIVARS *const ivars = PolyDelReader_IVARS(self);
    SeriesMatcher *deletions = NULL;
    if (ivars->del_count) {
        uint32_t num_readers = VA_Get_Size(ivars->readers);
        VArray *matchers = VA_new(num_readers);
        for (uint32_t i = 0; i < num_readers; i++) {
            DeletionsReader *reader
                = (DeletionsReader*)VA_Fetch(ivars->readers, i);
            Matcher *matcher = DelReader_Iterator(reader);
            if (matcher) { VA_Store(matchers, i, (Obj*)matcher); }
        }
        deletions = SeriesMatcher_new(matchers, ivars->offsets);
        DECREF(matchers);
    }
    return (Matcher*)deletions;
}

DefaultDeletionsReader*
DefDelReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
                 VArray *segments, int32_t seg_tick) {
    DefaultDeletionsReader *self
        = (DefaultDeletionsReader*)Class_Make_Obj(DEFAULTDELETIONSREADER);
    return DefDelReader_init(self, schema, folder, snapshot, segments,
                             seg_tick);
}

DefaultDeletionsReader*
DefDelReader_init(DefaultDeletionsReader *self, Schema *schema,
                  Folder *folder, Snapshot *snapshot, VArray *segments,
                  int32_t seg_tick) {
    DelReader_init((DeletionsReader*)self, schema, folder, snapshot, segments,
                   seg_tick);
    DefaultDeletionsReaderIVARS *const ivars = DefDelReader_IVARS(self);
    DefDelReader_Read_Deletions(self);
    if (!ivars->deldocs) {
        ivars->del_count = 0;
        ivars->deldocs   = BitVec_new(0);
    }
    return self;
}

void
DefDelReader_Close_IMP(DefaultDeletionsReader *self) {
    DefaultDeletionsReaderIVARS *const ivars = DefDelReader_IVARS(self);
    DECREF(ivars->deldocs);
    ivars->deldocs = NULL;
}

void
DefDelReader_Destroy_IMP(DefaultDeletionsReader *self) {
    DefaultDeletionsReaderIVARS *const ivars = DefDelReader_IVARS(self);
    DECREF(ivars->deldocs);
    SUPER_DESTROY(self, DEFAULTDELETIONSREADER);
}

BitVector*
DefDelReader_Read_Deletions_IMP(DefaultDeletionsReader *self) {
    DefaultDeletionsReaderIVARS *const ivars = DefDelReader_IVARS(self);
    VArray  *segments    = DefDelReader_Get_Segments(self);
    Segment *segment     = DefDelReader_Get_Segment(self);
    String  *my_seg_name = Seg_Get_Name(segment);
    String  *del_file    = NULL;
    int32_t  del_count   = 0;

    // Start with deletions files in the most recently added segments and work
    // backwards.  The first one we find which addresses our segment is the
    // one we need.
    for (int32_t i = VA_Get_Size(segments) - 1; i >= 0; i--) {
        Segment *other_seg = (Segment*)VA_Fetch(segments, i);
        Hash *metadata
            = (Hash*)Seg_Fetch_Metadata_Utf8(other_seg, "deletions", 9);
        if (metadata) {
            Hash *files = (Hash*)CERTIFY(
                              Hash_Fetch_Utf8(metadata, "files", 5), HASH);
            Hash *seg_files_data
                = (Hash*)Hash_Fetch(files, (Obj*)my_seg_name);
            if (seg_files_data) {
                Obj *count = (Obj*)CERTIFY(
                                 Hash_Fetch_Utf8(seg_files_data, "count", 5),
                                 OBJ);
                del_count = (int32_t)Obj_To_I64(count);
                del_file  = (String*)CERTIFY(
                                Hash_Fetch_Utf8(seg_files_data, "filename", 8),
                                STRING);
                break;
            }
        }
    }

    DECREF(ivars->deldocs);
    if (del_file) {
        ivars->deldocs = (BitVector*)BitVecDelDocs_new(ivars->folder, del_file);
        ivars->del_count = del_count;
    }
    else {
        ivars->deldocs = NULL;
        ivars->del_count = 0;
    }

    return ivars->deldocs;
}

Matcher*
DefDelReader_Iterator_IMP(DefaultDeletionsReader *self) {
    DefaultDeletionsReaderIVARS *const ivars = DefDelReader_IVARS(self);
    return (Matcher*)BitVecMatcher_new(ivars->deldocs);
}

int32_t
DefDelReader_Del_Count_IMP(DefaultDeletionsReader *self) {
    return DefDelReader_IVARS(self)->del_count;
}


