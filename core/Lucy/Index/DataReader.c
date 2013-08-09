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

#define C_LUCY_DATAREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/DataReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"

DataReader*
DataReader_init(DataReader *self, Schema *schema, Folder *folder,
                Snapshot *snapshot, VArray *segments, int32_t seg_tick) {
    DataReaderIVARS *const ivars = DataReader_IVARS(self);
    ivars->schema   = (Schema*)INCREF(schema);
    ivars->folder   = (Folder*)INCREF(folder);
    ivars->snapshot = (Snapshot*)INCREF(snapshot);
    ivars->segments = (VArray*)INCREF(segments);
    ivars->seg_tick = seg_tick;
    if (seg_tick != -1) {
        if (!segments) {
            THROW(ERR, "No segments array provided, but seg_tick is %i32",
                  seg_tick);
        }
        else {
            Segment *segment = (Segment*)VA_Fetch(segments, seg_tick);
            if (!segment) {
                THROW(ERR, "No segment at seg_tick %i32", seg_tick);
            }
            ivars->segment = (Segment*)INCREF(segment);
        }
    }
    else {
        ivars->segment = NULL;
    }

    ABSTRACT_CLASS_CHECK(self, DATAREADER);
    return self;
}

void
DataReader_Destroy_IMP(DataReader *self) {
    DataReaderIVARS *const ivars = DataReader_IVARS(self);
    DECREF(ivars->schema);
    DECREF(ivars->folder);
    DECREF(ivars->snapshot);
    DECREF(ivars->segments);
    DECREF(ivars->segment);
    SUPER_DESTROY(self, DATAREADER);
}

Schema*
DataReader_Get_Schema_IMP(DataReader *self) {
    return DataReader_IVARS(self)->schema;
}

Folder*
DataReader_Get_Folder_IMP(DataReader *self) {
    return DataReader_IVARS(self)->folder;
}

Snapshot*
DataReader_Get_Snapshot_IMP(DataReader *self) {
    return DataReader_IVARS(self)->snapshot;
}

VArray*
DataReader_Get_Segments_IMP(DataReader *self) {
    return DataReader_IVARS(self)->segments;
}

int32_t
DataReader_Get_Seg_Tick_IMP(DataReader *self) {
    return DataReader_IVARS(self)->seg_tick;
}

Segment*
DataReader_Get_Segment_IMP(DataReader *self) {
    return DataReader_IVARS(self)->segment;
}


