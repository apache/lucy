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
    SegWriter *self = (SegWriter*)Class_Make_Obj(SEGWRITER);
    return SegWriter_init(self, schema, snapshot, segment, polyreader);
}

SegWriter*
SegWriter_init(SegWriter *self, Schema *schema, Snapshot *snapshot,
               Segment *segment, PolyReader *polyreader) {
    Architecture *arch   = Schema_Get_Architecture(schema);
    DataWriter_init((DataWriter*)self, schema, snapshot, segment, polyreader);
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    ivars->by_api   = Hash_new(0);
    ivars->inverter = Inverter_new(schema, segment);
    ivars->writers  = VA_new(16);
    Arch_Init_Seg_Writer(arch, self);
    return self;
}

void
SegWriter_Destroy_IMP(SegWriter *self) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    DECREF(ivars->inverter);
    DECREF(ivars->writers);
    DECREF(ivars->by_api);
    DECREF(ivars->del_writer);
    SUPER_DESTROY(self, SEGWRITER);
}

void
SegWriter_Register_IMP(SegWriter *self, String *api,
                       DataWriter *component) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    CERTIFY(component, DATAWRITER);
    if (Hash_Fetch(ivars->by_api, (Obj*)api)) {
        THROW(ERR, "API %o already registered", api);
    }
    Hash_Store(ivars->by_api, (Obj*)api, (Obj*)component);
}

Obj*
SegWriter_Fetch_IMP(SegWriter *self, String *api) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    return Hash_Fetch(ivars->by_api, (Obj*)api);
}

void
SegWriter_Add_Writer_IMP(SegWriter *self, DataWriter *writer) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    VA_Push(ivars->writers, (Obj*)writer);
}

void
SegWriter_Prep_Seg_Dir_IMP(SegWriter *self) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    Folder *folder   = SegWriter_Get_Folder(self);
    String *seg_name = Seg_Get_Name(ivars->segment);

    // Clear stale segment files from crashed indexing sessions.
    if (Folder_Exists(folder, seg_name)) {
        bool result = Folder_Delete_Tree(folder, seg_name);
        if (!result) {
            THROW(ERR, "Couldn't completely remove '%o'", seg_name);
        }
    }

    // Create the segment directory.
    bool result = Folder_MkDir(folder, seg_name);
    if (!result) { RETHROW(INCREF(Err_get_error())); }
}

void
SegWriter_Add_Doc_IMP(SegWriter *self, Doc *doc, float boost) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    int32_t doc_id = (int32_t)Seg_Increment_Count(ivars->segment, 1);
    Inverter_Invert_Doc(ivars->inverter, doc);
    Inverter_Set_Boost(ivars->inverter, boost);
    SegWriter_Add_Inverted_Doc(self, ivars->inverter, doc_id);
}

void
SegWriter_Add_Inverted_Doc_IMP(SegWriter *self, Inverter *inverter,
                               int32_t doc_id) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(ivars->writers, i);
        DataWriter_Add_Inverted_Doc(writer, inverter, doc_id);
    }
}

// Adjust current doc id. We create our own doc_count rather than rely on
// SegReader's number because the DeletionsWriter and the SegReader are
// probably out of sync.
static void
S_adjust_doc_id(SegWriter *self, SegReader *reader, I32Array *doc_map) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    uint32_t doc_count = SegReader_Doc_Max(reader);
    for (uint32_t i = 1, max = I32Arr_Get_Size(doc_map); i < max; i++) {
        if (I32Arr_Get(doc_map, i) == 0) { doc_count--; }
    }
    Seg_Increment_Count(ivars->segment, doc_count);
}

void
SegWriter_Add_Segment_IMP(SegWriter *self, SegReader *reader,
                          I32Array *doc_map) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);

    // Bulk add the slab of documents to the various writers.
    for (uint32_t i = 0, max = VA_Get_Size(ivars->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(ivars->writers, i);
        DataWriter_Add_Segment(writer, reader, doc_map);
    }

    // Bulk add the segment to the DeletionsWriter, so that it can merge
    // previous segment files as necessary.
    DelWriter_Add_Segment(ivars->del_writer, reader, doc_map);

    // Adust the document id.
    S_adjust_doc_id(self, reader, doc_map);
}

void
SegWriter_Merge_Segment_IMP(SegWriter *self, SegReader *reader,
                            I32Array *doc_map) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    Snapshot *snapshot = SegWriter_Get_Snapshot(self);
    String   *seg_name = Seg_Get_Name(SegReader_Get_Segment(reader));

    // Have all the sub-writers merge the segment.
    for (uint32_t i = 0, max = VA_Get_Size(ivars->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(ivars->writers, i);
        DataWriter_Merge_Segment(writer, reader, doc_map);
    }
    DelWriter_Merge_Segment(ivars->del_writer, reader, doc_map);

    // Remove seg directory from snapshot.
    Snapshot_Delete_Entry(snapshot, seg_name);

    // Adust the document id.
    S_adjust_doc_id(self, reader, doc_map);
}

void
SegWriter_Delete_Segment_IMP(SegWriter *self, SegReader *reader) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    Snapshot *snapshot = SegWriter_Get_Snapshot(self);
    String   *seg_name = Seg_Get_Name(SegReader_Get_Segment(reader));

    // Have all the sub-writers delete the segment.
    for (uint32_t i = 0, max = VA_Get_Size(ivars->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(ivars->writers, i);
        DataWriter_Delete_Segment(writer, reader);
    }
    DelWriter_Delete_Segment(ivars->del_writer, reader);

    // Remove seg directory from snapshot.
    Snapshot_Delete_Entry(snapshot, seg_name);
}

void
SegWriter_Finish_IMP(SegWriter *self) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    String *seg_name = Seg_Get_Name(ivars->segment);

    // Finish off children.
    for (uint32_t i = 0, max = VA_Get_Size(ivars->writers); i < max; i++) {
        DataWriter *writer = (DataWriter*)VA_Fetch(ivars->writers, i);
        DataWriter_Finish(writer);
    }

    // Write segment metadata and add the segment directory to the snapshot.
    Snapshot *snapshot = SegWriter_Get_Snapshot(self);
    String *segmeta_filename = Str_newf("%o/segmeta.json", seg_name);
    Seg_Write_File(ivars->segment, ivars->folder);
    Snapshot_Add_Entry(snapshot, seg_name);
    DECREF(segmeta_filename);

    // Collapse segment files into compound file.
    Folder_Consolidate(ivars->folder, seg_name);
}

void
SegWriter_Add_Data_Writer_IMP(SegWriter *self, DataWriter *writer) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    VA_Push(ivars->writers, (Obj*)writer);
}

void
SegWriter_Set_Del_Writer_IMP(SegWriter *self, DeletionsWriter *del_writer) {
    SegWriterIVARS *const ivars = SegWriter_IVARS(self);
    DECREF(ivars->del_writer);
    ivars->del_writer = (DeletionsWriter*)INCREF(del_writer);
}

DeletionsWriter*
SegWriter_Get_Del_Writer_IMP(SegWriter *self) {
    return SegWriter_IVARS(self)->del_writer;
}


