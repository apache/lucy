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
    self->snapshot   = (Snapshot*)INCREF(snapshot);
    self->segment    = (Segment*)INCREF(segment);
    self->polyreader = (PolyReader*)INCREF(polyreader);
    self->schema     = (Schema*)INCREF(schema);
    self->folder     = (Folder*)INCREF(PolyReader_Get_Folder(polyreader));
    ABSTRACT_CLASS_CHECK(self, DATAWRITER);
    return self;
}

void
DataWriter_destroy(DataWriter *self) {
    DECREF(self->snapshot);
    DECREF(self->segment);
    DECREF(self->polyreader);
    DECREF(self->schema);
    DECREF(self->folder);
    SUPER_DESTROY(self, DATAWRITER);
}

Snapshot*
DataWriter_get_snapshot(DataWriter *self) {
    return self->snapshot;
}

Segment*
DataWriter_get_segment(DataWriter *self) {
    return self->segment;
}

PolyReader*
DataWriter_get_polyreader(DataWriter *self) {
    return self->polyreader;
}

Schema*
DataWriter_get_schema(DataWriter *self) {
    return self->schema;
}

Folder*
DataWriter_get_folder(DataWriter *self) {
    return self->folder;
}

void
DataWriter_delete_segment(DataWriter *self, SegReader *reader) {
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
}

void
DataWriter_merge_segment(DataWriter *self, SegReader *reader,
                         I32Array *doc_map) {
    DataWriter_Add_Segment(self, reader, doc_map);
    DataWriter_Delete_Segment(self, reader);
}

Hash*
DataWriter_metadata(DataWriter *self) {
    Hash *metadata = Hash_new(0);
    Hash_Store_Str(metadata, "format", 6,
                   (Obj*)CB_newf("%i32", DataWriter_Format(self)));
    return metadata;
}


