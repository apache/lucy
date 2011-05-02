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

#define C_LUCY_SEGWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SegWriter.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/Inverter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Architecture.h"

SegWriter*
SegWriter_new(Schema *schema, Snapshot *snapshot, Segment *segment,
              PolyReader *polyreader) {
    SegWriter *self = (SegWriter*)VTable_Make_Obj(SEGWRITER);
    return SegWriter_init(self, schema, snapshot, segment, polyreader);
}

SegWriter*
SegWriter_init(SegWriter *self, Schema *schema, Snapshot *snapshot,
               Segment *segment, PolyReader *polyreader) {
    Architecture *arch   = Schema_Get_Architecture(schema);
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    self->by_api   = Hash_new(0);
    self->inverter = Inverter_new(schema, segment);
    self->writers  = VA_new(16);
    Arch_Init_Seg_Writer(arch, self);
    return self;
}

void
SegWriter_destroy(SegWriter *self) {
    DECREF(self->inverter);
    DECREF(self->writers);
    DECREF(self->by_api);
    DECREF(self->del_writer);
    SUPER_DESTROY(self, SEGWRITER);
}

void
SegWriter_register(SegWriter *self, const CharBuf *api,
                   DataWriter *component) {
    CERTIFY(component, DATAWRITER);
    if (Hash_Fetch(self->by_api, (Obj*)api)) {
        THROW(ERR, "API %o already registered", api);
    }
    Hash_Store(self->by_api, (Obj*)api, (Obj*)component);
}

Obj*
SegWriter_fetch(SegWriter *self, const CharBuf *api) {
    return Hash_Fetch(self->by_api, (Obj*)api);
}

void
SegWriter_add_writer(SegWriter *self, DataWriter *writer) {
    VA_Push(self->writers, (Obj*)writer);
}

void
SegWriter_prep_seg_dir(SegWriter *self) {
    Folder  *folder   = SegWriter_Get_Folder(self);
    CharBuf *seg_name = Seg_Get_Name(self->segment);

    // Clear stale segment files from crashed indexing sessions.
    if (Folder_Exists(folder, seg_name)) {
        bool_t result = Folder_Delete_Tree(folder, seg_name);
        if (!result) {
            THROW(ERR, "Couldn't completely remove '%o'", seg_name);
        }
    }

    {
        // Create the segment directory.
        bool_t result = Folder_MkDir(folder, seg_name);
        if (!result) { RETHROW(INCREF(Err_get_error())); }
    }
}

void
SegWriter_add_doc(SegWriter *self, Doc *doc, float boost) {
    int32_t doc_id = (int32_t)Seg_Increment_Count(self->segment, 1);
    Inverter_Invert_Doc(self->inverter, doc);
    Inverter_Set_Boost(self->inverter, boost);
    SegWriter_Add_Inverted_Doc(self, self->inverter, doc_id);
}

void
SegWriter_add_inverted_doc(SegWriter *self, Inverter *inverter,
                           int32_t doc_id) {
    uint32_t i, max;
    for (i = 0, max = VA_Get_Size(self->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(self->writers, i);
        DataWriter_Add_Inverted_Doc(writer, inverter, doc_id);
    }
}

// Adjust current doc id. We create our own doc_count rather than rely on
// SegReader's number because the DeletionsWriter and the SegReader are
// probably out of sync.
static void
S_adjust_doc_id(SegWriter *self, SegReader *reader, I32Array *doc_map) {
    uint32_t doc_count = SegReader_Doc_Max(reader);
    uint32_t i, max;
    for (i = 1, max = I32Arr_Get_Size(doc_map); i < max; i++) {
        if (I32Arr_Get(doc_map, i) == 0) { doc_count--; }
    }
    Seg_Increment_Count(self->segment, doc_count);
}

void
SegWriter_add_segment(SegWriter *self, SegReader *reader, I32Array *doc_map) {
    uint32_t i, max;

    // Bulk add the slab of documents to the various writers.
    for (i = 0, max = VA_Get_Size(self->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(self->writers, i);
        DataWriter_Add_Segment(writer, reader, doc_map);
    }

    // Bulk add the segment to the DeletionsWriter, so that it can merge
    // previous segment files as necessary.
    DelWriter_Add_Segment(self->del_writer, reader, doc_map);

    // Adust the document id.
    S_adjust_doc_id(self, reader, doc_map);
}

void
SegWriter_merge_segment(SegWriter *self, SegReader *reader,
                        I32Array *doc_map) {
    Snapshot *snapshot = SegWriter_Get_Snapshot(self);
    CharBuf  *seg_name = Seg_Get_Name(SegReader_Get_Segment(reader));
    uint32_t i, max;

    // Have all the sub-writers merge the segment.
    for (i = 0, max = VA_Get_Size(self->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(self->writers, i);
        DataWriter_Merge_Segment(writer, reader, doc_map);
    }
    DelWriter_Merge_Segment(self->del_writer, reader, doc_map);

    // Remove seg directory from snapshot.
    Snapshot_Delete_Entry(snapshot, seg_name);

    // Adust the document id.
    S_adjust_doc_id(self, reader, doc_map);
}

void
SegWriter_delete_segment(SegWriter *self, SegReader *reader) {
    Snapshot *snapshot = SegWriter_Get_Snapshot(self);
    CharBuf  *seg_name = Seg_Get_Name(SegReader_Get_Segment(reader));
    uint32_t i, max;

    // Have all the sub-writers delete the segment.
    for (i = 0, max = VA_Get_Size(self->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(self->writers, i);
        DataWriter_Delete_Segment(writer, reader);
    }
    DelWriter_Delete_Segment(self->del_writer, reader);

    // Remove seg directory from snapshot.
    Snapshot_Delete_Entry(snapshot, seg_name);
}

void
SegWriter_finish(SegWriter *self) {
    CharBuf *seg_name = Seg_Get_Name(self->segment);
    uint32_t i, max;

    // Finish off children.
    for (i = 0, max = VA_Get_Size(self->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(self->writers, i);
        DataWriter_Finish(writer);
    }

    // Write segment metadata and add the segment directory to the snapshot.
    {
        Snapshot *snapshot = SegWriter_Get_Snapshot(self);
        CharBuf *segmeta_filename = CB_newf("%o/segmeta.json", seg_name);
        Seg_Write_File(self->segment, self->folder);
        Snapshot_Add_Entry(snapshot, seg_name);
        DECREF(segmeta_filename);
    }

    // Collapse segment files into compound file.
    Folder_Consolidate(self->folder, seg_name);
}

void
SegWriter_add_data_writer(SegWriter *self, DataWriter *writer) {
    VA_Push(self->writers, (Obj*)writer);
}

void
SegWriter_set_del_writer(SegWriter *self, DeletionsWriter *del_writer) {
    DECREF(self->del_writer);
    self->del_writer = (DeletionsWriter*)INCREF(del_writer);
}

DeletionsWriter*
SegWriter_get_del_writer(SegWriter *self) {
    return self->del_writer;
}


