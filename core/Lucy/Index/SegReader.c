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

// Try to initialize all sub-readers.
static void
S_try_init_components(void *context);

SegReader*
SegReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
              VArray *segments, int32_t seg_tick) {
    SegReader *self = (SegReader*)Class_Make_Obj(SEGREADER);
    return SegReader_init(self, schema, folder, snapshot, segments, seg_tick);
}

SegReader*
SegReader_init(SegReader *self, Schema *schema, Folder *folder,
               Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    Segment *segment;

    IxReader_init((IndexReader*)self, schema, folder, snapshot, segments,
                  seg_tick, NULL);
    SegReaderIVARS *const ivars = SegReader_IVARS(self);
    segment = SegReader_Get_Segment(self);

    ivars->doc_max    = (int32_t)Seg_Get_Count(segment);
    ivars->seg_name   = (String*)INCREF(Seg_Get_Name(segment));
    ivars->seg_num    = Seg_Get_Number(segment);
    Err *error = Err_trap(S_try_init_components, self);
    if (error) {
        // An error occurred, so clean up self and rethrow the exception.
        DECREF(self);
        RETHROW(error);
    }

    DeletionsReader *del_reader
        = (DeletionsReader*)Hash_Fetch(
              ivars->components, (Obj*)Class_Get_Name(DELETIONSREADER));
    ivars->del_count = del_reader ? DelReader_Del_Count(del_reader) : 0;

    return self;
}

static void
S_try_init_components(void *context) {
    SegReader *self = (SegReader*)context;
    Schema *schema = SegReader_Get_Schema(self);
    Architecture *arch = Schema_Get_Architecture(schema);
    Arch_Init_Seg_Reader(arch, self);
}

void
SegReader_Destroy_IMP(SegReader *self) {
    SegReaderIVARS *const ivars = SegReader_IVARS(self);
    DECREF(ivars->seg_name);
    SUPER_DESTROY(self, SEGREADER);
}

void
SegReader_Register_IMP(SegReader *self, String *api,
                       DataReader *component) {
    SegReaderIVARS *const ivars = SegReader_IVARS(self);
    if (Hash_Fetch(ivars->components, (Obj*)api)) {
        THROW(ERR, "Interface '%o' already registered");
    }
    CERTIFY(component, DATAREADER);
    Hash_Store(ivars->components, (Obj*)api, (Obj*)component);
}

String*
SegReader_Get_Seg_Name_IMP(SegReader *self) {
    return SegReader_IVARS(self)->seg_name;
}

int64_t
SegReader_Get_Seg_Num_IMP(SegReader *self) {
    return SegReader_IVARS(self)->seg_num;
}

int32_t
SegReader_Del_Count_IMP(SegReader *self) {
    return SegReader_IVARS(self)->del_count;
}

int32_t
SegReader_Doc_Max_IMP(SegReader *self) {
    return SegReader_IVARS(self)->doc_max;
}

int32_t
SegReader_Doc_Count_IMP(SegReader *self) {
    SegReaderIVARS *const ivars = SegReader_IVARS(self);
    return ivars->doc_max - ivars->del_count;
}

I32Array*
SegReader_Offsets_IMP(SegReader *self) {
    int32_t *ints = (int32_t*)CALLOCATE(1, sizeof(int32_t));
    UNUSED_VAR(self);
    return I32Arr_new_steal(ints, 1);
}

VArray*
SegReader_Seg_Readers_IMP(SegReader *self) {
    VArray *seg_readers = VA_new(1);
    VA_Push(seg_readers, INCREF(self));
    return seg_readers;
}


