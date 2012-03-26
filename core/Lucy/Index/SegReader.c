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

#define C_LUCY_SEGREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/DocReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Store/Folder.h"

SegReader*
SegReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
              VArray *segments, int32_t seg_tick) {
    SegReader *self = (SegReader*)VTable_Make_Obj(SEGREADER);
    return SegReader_init(self, schema, folder, snapshot, segments, seg_tick);
}

SegReader*
SegReader_init(SegReader *self, Schema *schema, Folder *folder,
               Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    CharBuf *mess;
    Segment *segment;

    IxReader_init((IndexReader*)self, schema, folder, snapshot, segments,
                  seg_tick, NULL);
    segment = SegReader_Get_Segment(self);

    self->doc_max    = (int32_t)Seg_Get_Count(segment);
    self->seg_name   = (CharBuf*)INCREF(Seg_Get_Name(segment));
    self->seg_num    = Seg_Get_Number(segment);
    mess = SegReader_Try_Init_Components(self);
    if (mess) {
        // An error occurred, so clean up self and throw an exception.
        DECREF(self);
        Err_throw_mess(ERR, mess);
    }
    {
        DeletionsReader *del_reader
            = (DeletionsReader*)Hash_Fetch(
                  self->components, (Obj*)VTable_Get_Name(DELETIONSREADER));
        self->del_count = del_reader ? DelReader_Del_Count(del_reader) : 0;
    }
    return self;
}

void
SegReader_destroy(SegReader *self) {
    DECREF(self->seg_name);
    SUPER_DESTROY(self, SEGREADER);
}

void
SegReader_register(SegReader *self, const CharBuf *api,
                   DataReader *component) {
    if (Hash_Fetch(self->components, (Obj*)api)) {
        THROW(ERR, "Interface '%o' already registered");
    }
    CERTIFY(component, DATAREADER);
    Hash_Store(self->components, (Obj*)api, (Obj*)component);
}

CharBuf*
SegReader_get_seg_name(SegReader *self) {
    return self->seg_name;
}

int64_t
SegReader_get_seg_num(SegReader *self) {
    return self->seg_num;
}

int32_t
SegReader_del_count(SegReader *self) {
    return self->del_count;
}

int32_t
SegReader_doc_max(SegReader *self) {
    return self->doc_max;
}

int32_t
SegReader_doc_count(SegReader *self) {
    return self->doc_max - self->del_count;
}

I32Array*
SegReader_offsets(SegReader *self) {
    int32_t *ints = (int32_t*)CALLOCATE(1, sizeof(int32_t));
    UNUSED_VAR(self);
    return I32Arr_new_steal(ints, 1);
}

VArray*
SegReader_seg_readers(SegReader *self) {
    VArray *seg_readers = VA_new(1);
    VA_Push(seg_readers, INCREF(self));
    return seg_readers;
}


