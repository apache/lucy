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

#define C_LUCY_DATAWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/DataWriter.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Store/Folder.h"

DataWriter*
DataWriter_init(DataWriter *self, Schema *schema, Snapshot *snapshot,
                Segment *segment, PolyReader *polyreader) {
    DataWriterIVARS *const ivars = DataWriter_IVARS(self);
    ivars->snapshot   = (Snapshot*)INCREF(snapshot);
    ivars->segment    = (Segment*)INCREF(segment);
    ivars->polyreader = (PolyReader*)INCREF(polyreader);
    ivars->schema     = (Schema*)INCREF(schema);
    ivars->folder     = (Folder*)INCREF(PolyReader_Get_Folder(polyreader));
    ABSTRACT_CLASS_CHECK(self, DATAWRITER);
    return self;
}

void
DataWriter_Destroy_IMP(DataWriter *self) {
    DataWriterIVARS *const ivars = DataWriter_IVARS(self);
    DECREF(ivars->snapshot);
    DECREF(ivars->segment);
    DECREF(ivars->polyreader);
    DECREF(ivars->schema);
    DECREF(ivars->folder);
    SUPER_DESTROY(self, DATAWRITER);
}

Snapshot*
DataWriter_Get_Snapshot_IMP(DataWriter *self) {
    return DataWriter_IVARS(self)->snapshot;
}

Segment*
DataWriter_Get_Segment_IMP(DataWriter *self) {
    return DataWriter_IVARS(self)->segment;
}

PolyReader*
DataWriter_Get_PolyReader_IMP(DataWriter *self) {
    return DataWriter_IVARS(self)->polyreader;
}

Schema*
DataWriter_Get_Schema_IMP(DataWriter *self) {
    return DataWriter_IVARS(self)->schema;
}

Folder*
DataWriter_Get_Folder_IMP(DataWriter *self) {
    return DataWriter_IVARS(self)->folder;
}

void
DataWriter_Delete_Segment_IMP(DataWriter *self, SegReader *reader) {
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
DataWriter_Merge_Segment_IMP(DataWriter *self, SegReader *reader,
                             I32Array *doc_map) {
    DataWriter_Add_Segment(self, reader, doc_map);
    DataWriter_Delete_Segment(self, reader);
}

Hash*
DataWriter_Metadata_IMP(DataWriter *self) {
    Hash *metadata = Hash_new(0);
    Hash_Store_Utf8(metadata, "format", 6,
                    (Obj*)Str_newf("%i32", DataWriter_Format(self)));
    return metadata;
}


