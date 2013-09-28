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

#define C_LUCY_INDEXREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/IndexReader.h"
#include "Lucy/Index/DocVector.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/Lock.h"

IndexReader*
IxReader_open(Obj *index, Snapshot *snapshot, IndexManager *manager) {
    return IxReader_do_open(NULL, index, snapshot, manager);
}

IndexReader*
IxReader_do_open(IndexReader *temp_self, Obj *index, Snapshot *snapshot,
                 IndexManager *manager) {
    PolyReader *polyreader = PolyReader_open(index, snapshot, manager);
    if (!VA_Get_Size(PolyReader_Get_Seg_Readers(polyreader))) {
        THROW(ERR, "Index doesn't seem to contain any data");
    }
    DECREF(temp_self);
    return (IndexReader*)polyreader;
}

IndexReader*
IxReader_init(IndexReader *self, Schema *schema, Folder *folder,
              Snapshot *snapshot, VArray *segments, int32_t seg_tick,
              IndexManager *manager) {
    snapshot = snapshot ? (Snapshot*)INCREF(snapshot) : Snapshot_new();
    DataReader_init((DataReader*)self, schema, folder, snapshot, segments,
                    seg_tick);
    DECREF(snapshot);
    IndexReaderIVARS *const ivars = IxReader_IVARS(self);
    ivars->components     = Hash_new(0);
    ivars->read_lock      = NULL;
    ivars->deletion_lock  = NULL;
    if (manager) {
        ivars->manager = (IndexManager*)INCREF(manager);
        IxManager_Set_Folder(ivars->manager, ivars->folder);
    }
    else {
        ivars->manager = NULL;
    }
    return self;
}

void
IxReader_Close_IMP(IndexReader *self) {
    IndexReaderIVARS *const ivars = IxReader_IVARS(self);
    if (ivars->components) {
        String *key;
        DataReader *component;
        Hash_Iterate(ivars->components);
        while (Hash_Next(ivars->components, (Obj**)&key,
                         (Obj**)&component)
              ) {
            if (Obj_Is_A((Obj*)component, DATAREADER)) {
                DataReader_Close(component);
            }
        }
        Hash_Clear(ivars->components);
    }
    if (ivars->read_lock) {
        Lock_Release(ivars->read_lock);
        DECREF(ivars->read_lock);
        ivars->read_lock = NULL;
    }
}

void
IxReader_Destroy_IMP(IndexReader *self) {
    IndexReaderIVARS *const ivars = IxReader_IVARS(self);
    DECREF(ivars->components);
    if (ivars->read_lock) {
        Lock_Release(ivars->read_lock);
        DECREF(ivars->read_lock);
    }
    DECREF(ivars->manager);
    DECREF(ivars->deletion_lock);
    SUPER_DESTROY(self, INDEXREADER);
}

Hash*
IxReader_Get_Components_IMP(IndexReader *self) {
    return IxReader_IVARS(self)->components;
}

DataReader*
IxReader_Obtain_IMP(IndexReader *self, String *api) {
    IndexReaderIVARS *const ivars = IxReader_IVARS(self);
    DataReader *component
        = (DataReader*)Hash_Fetch(ivars->components, (Obj*)api);
    if (!component) {
        THROW(ERR, "No component registered for '%o'", api);
    }
    return component;
}

DataReader*
IxReader_Fetch_IMP(IndexReader *self, String *api) {
    IndexReaderIVARS *const ivars = IxReader_IVARS(self);
    return (DataReader*)Hash_Fetch(ivars->components, (Obj*)api);
}


