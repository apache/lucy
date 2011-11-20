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

#define C_LUCY_POLYREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/PolyReader.h"
#include "Lucy/Document/HitDoc.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/FieldType.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Util/Json.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/StringHelper.h"

// Obtain/release read locks and commit locks.  If self->manager is
// NULL, do nothing.
static void
S_obtain_read_lock(PolyReader *self, const CharBuf *snapshot_filename);
static void
S_obtain_deletion_lock(PolyReader *self);
static void
S_release_read_lock(PolyReader *self);
static void
S_release_deletion_lock(PolyReader *self);

static Folder*
S_derive_folder(Obj *index);

PolyReader*
PolyReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
               IndexManager *manager, VArray *sub_readers) {
    PolyReader *self = (PolyReader*)VTable_Make_Obj(POLYREADER);
    return PolyReader_init(self, schema, folder, snapshot, manager,
                           sub_readers);
}

PolyReader*
PolyReader_open(Obj *index, Snapshot *snapshot, IndexManager *manager) {
    PolyReader *self = (PolyReader*)VTable_Make_Obj(POLYREADER);
    return PolyReader_do_open(self, index, snapshot, manager);
}

static Obj*
S_first_non_null(VArray *array) {
    for (uint32_t i = 0, max = VA_Get_Size(array); i < max; i++) {
        Obj *thing = VA_Fetch(array, i);
        if (thing) { return thing; }
    }
    return NULL;
}

static void
S_init_sub_readers(PolyReader *self, VArray *sub_readers) {
    uint32_t  num_sub_readers = VA_Get_Size(sub_readers);
    int32_t *starts = (int32_t*)MALLOCATE(num_sub_readers * sizeof(int32_t));
    Hash  *data_readers = Hash_new(0);

    DECREF(self->sub_readers);
    DECREF(self->offsets);
    self->sub_readers       = (VArray*)INCREF(sub_readers);

    // Accumulate doc_max, subreader start offsets, and DataReaders.
    self->doc_max = 0;
    for (uint32_t i = 0; i < num_sub_readers; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(sub_readers, i);
        Hash *components = SegReader_Get_Components(seg_reader);
        CharBuf *api;
        DataReader *component;
        starts[i] = self->doc_max;
        self->doc_max += SegReader_Doc_Max(seg_reader);
        Hash_Iterate(components);
        while (Hash_Next(components, (Obj**)&api, (Obj**)&component)) {
            VArray *readers = (VArray*)Hash_Fetch(data_readers, (Obj*)api);
            if (!readers) {
                readers = VA_new(num_sub_readers);
                Hash_Store(data_readers, (Obj*)api, (Obj*)readers);
            }
            VA_Store(readers, i, INCREF(component));
        }
    }
    self->offsets = I32Arr_new_steal(starts, num_sub_readers);

    CharBuf *api;
    VArray  *readers;
    Hash_Iterate(data_readers);
    while (Hash_Next(data_readers, (Obj**)&api, (Obj**)&readers)) {
        DataReader *datareader
            = (DataReader*)CERTIFY(S_first_non_null(readers), DATAREADER);
        DataReader *aggregator
            = DataReader_Aggregator(datareader, readers, self->offsets);
        if (aggregator) {
            CERTIFY(aggregator, DATAREADER);
            Hash_Store(self->components, (Obj*)api, (Obj*)aggregator);
        }
    }
    DECREF(data_readers);

    DeletionsReader *del_reader
        = (DeletionsReader*)Hash_Fetch(
              self->components, (Obj*)VTable_Get_Name(DELETIONSREADER));
    self->del_count = del_reader ? DelReader_Del_Count(del_reader) : 0;
}

PolyReader*
PolyReader_init(PolyReader *self, Schema *schema, Folder *folder,
                Snapshot *snapshot, IndexManager *manager,
                VArray *sub_readers) {
    self->doc_max    = 0;
    self->del_count  = 0;

    if (sub_readers) {
        uint32_t num_segs = VA_Get_Size(sub_readers);
        VArray *segments = VA_new(num_segs);
        for (uint32_t i = 0; i < num_segs; i++) {
            SegReader *seg_reader
                = (SegReader*)CERTIFY(VA_Fetch(sub_readers, i), SEGREADER);
            VA_Push(segments, INCREF(SegReader_Get_Segment(seg_reader)));
        }
        IxReader_init((IndexReader*)self, schema, folder, snapshot,
                      segments, -1, manager);
        DECREF(segments);
        S_init_sub_readers(self, sub_readers);
    }
    else {
        IxReader_init((IndexReader*)self, schema, folder, snapshot,
                      NULL, -1, manager);
        self->sub_readers = VA_new(0);
        self->offsets = I32Arr_new_steal(NULL, 0);
    }

    return self;
}

void
PolyReader_close(PolyReader *self) {
    PolyReader_close_t super_close
        = (PolyReader_close_t)SUPER_METHOD(POLYREADER, PolyReader, Close);
    for (uint32_t i = 0, max = VA_Get_Size(self->sub_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(self->sub_readers, i);
        SegReader_Close(seg_reader);
    }
    super_close(self);
}

void
PolyReader_destroy(PolyReader *self) {
    DECREF(self->sub_readers);
    DECREF(self->offsets);
    SUPER_DESTROY(self, POLYREADER);
}

Obj*
S_try_open_elements(PolyReader *self) {
    VArray   *files             = Snapshot_List(self->snapshot);
    Folder   *folder            = PolyReader_Get_Folder(self);
    uint32_t  num_segs          = 0;
    uint64_t  latest_schema_gen = 0;
    CharBuf  *schema_file       = NULL;

    // Find schema file, count segments.
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        CharBuf *entry = (CharBuf*)VA_Fetch(files, i);

        if (Seg_valid_seg_name(entry)) {
            num_segs++;
        }
        else if (CB_Starts_With_Str(entry, "schema_", 7)
                 && CB_Ends_With_Str(entry, ".json", 5)
                ) {
            uint64_t gen = IxFileNames_extract_gen(entry);
            if (gen > latest_schema_gen) {
                latest_schema_gen = gen;
                if (!schema_file) { schema_file = CB_Clone(entry); }
                else { CB_Mimic(schema_file, (Obj*)entry); }
            }
        }
    }

    // Read Schema.
    if (!schema_file) {
        CharBuf *mess = MAKE_MESS("Can't find a schema file.");
        DECREF(files);
        return (Obj*)mess;
    }
    else {
        Hash *dump = (Hash*)Json_slurp_json(folder, schema_file);
        if (dump) { // read file successfully
            DECREF(self->schema);
            self->schema = (Schema*)CERTIFY(
                               VTable_Load_Obj(SCHEMA, (Obj*)dump), SCHEMA);
            DECREF(dump);
            DECREF(schema_file);
            schema_file = NULL;
        }
        else {
            CharBuf *mess = MAKE_MESS("Failed to parse %o", schema_file);
            DECREF(schema_file);
            DECREF(files);
            return (Obj*)mess;
        }
    }

    VArray *segments = VA_new(num_segs);
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        CharBuf *entry = (CharBuf*)VA_Fetch(files, i);

        // Create a Segment for each segmeta.
        if (Seg_valid_seg_name(entry)) {
            int64_t seg_num = IxFileNames_extract_gen(entry);
            Segment *segment = Seg_new(seg_num);

            // Bail if reading the file fails (probably because it's been
            // deleted and a new snapshot file has been written so we need to
            // retry).
            if (Seg_Read_File(segment, folder)) {
                VA_Push(segments, (Obj*)segment);
            }
            else {
                CharBuf *mess = MAKE_MESS("Failed to read %o", entry);
                DECREF(segment);
                DECREF(segments);
                DECREF(files);
                return (Obj*)mess;
            }
        }
    }

    // Sort the segments by age.
    VA_Sort(segments, NULL, NULL);

    Obj *result = PolyReader_Try_Open_SegReaders(self, segments);
    DECREF(segments);
    DECREF(files);
    return result;

}

// For test suite.
CharBuf* PolyReader_race_condition_debug1 = NULL;
int32_t  PolyReader_debug1_num_passes     = 0;

PolyReader*
PolyReader_do_open(PolyReader *self, Obj *index, Snapshot *snapshot,
                   IndexManager *manager) {
    Folder   *folder   = S_derive_folder(index);
    uint64_t  last_gen = 0;

    PolyReader_init(self, NULL, folder, snapshot, manager, NULL);
    DECREF(folder);

    if (manager) { S_obtain_deletion_lock(self); }

    while (1) {
        CharBuf *target_snap_file;

        // If a Snapshot was supplied, use its file.
        if (snapshot) {
            target_snap_file = Snapshot_Get_Path(snapshot);
            if (!target_snap_file) {
                THROW(ERR, "Supplied snapshot objects must not be empty");
            }
            else {
                CB_Inc_RefCount(target_snap_file);
            }
        }
        else {
            // Otherwise, pick the most recent snap file.
            target_snap_file = IxFileNames_latest_snapshot(folder);

            // No snap file?  Looks like the index is empty.  We can stop now
            // and return NULL.
            if (!target_snap_file) { break; }
        }

        // Derive "generation" of this snapshot file from its name.
        uint64_t gen = IxFileNames_extract_gen(target_snap_file);

        // Get a read lock on the most recent snapshot file if indicated.
        if (manager) {
            S_obtain_read_lock(self, target_snap_file);
        }

        // Testing only.
        if (PolyReader_race_condition_debug1) {
            ZombieCharBuf *temp = ZCB_WRAP_STR("temp", 4);
            if (Folder_Exists(folder, (CharBuf*)temp)) {
                bool_t success = Folder_Rename(folder, (CharBuf*)temp,
                                               PolyReader_race_condition_debug1);
                if (!success) { RETHROW(INCREF(Err_get_error())); }
            }
            PolyReader_debug1_num_passes++;
        }

        // If a Snapshot object was passed in, the file has already been read.
        // If that's not the case, we must read the file we just picked.
        if (!snapshot) {
            CharBuf *error = PolyReader_try_read_snapshot(self->snapshot, folder,
                                                          target_snap_file);

            if (error) {
                S_release_read_lock(self);
                DECREF(target_snap_file);
                if (last_gen < gen) { // Index updated, so try again.
                    DECREF(error);
                    last_gen = gen;
                    continue;
                }
                else { // Real error.
                    if (manager) { S_release_deletion_lock(self); }
                    Err_throw_mess(ERR, error);
                }
            }
        }

        /* It's possible, though unlikely, for an Indexer to delete files
         * out from underneath us after the snapshot file is read but before
         * we've got SegReaders holding open all the required files.  If we
         * failed to open something, see if we can find a newer snapshot file.
         * If we can, then the exception was due to the race condition.  If
         * not, we have a real exception, so throw an error. */
        Obj *result = S_try_open_elements(self);
        if (Obj_Is_A(result, CHARBUF)) { // Error occurred.
            S_release_read_lock(self);
            DECREF(target_snap_file);
            if (last_gen < gen) { // Index updated, so try again.
                DECREF(result);
                last_gen = gen;
            }
            else { // Real error.
                if (manager) { S_release_deletion_lock(self); }
                Err_throw_mess(ERR, (CharBuf*)result);
            }
        }
        else { // Succeeded.
            S_init_sub_readers(self, (VArray*)result);
            DECREF(result);
            DECREF(target_snap_file);
            break;
        }
    }

    if (manager) { S_release_deletion_lock(self); }

    return self;
}

static Folder*
S_derive_folder(Obj *index) {
    Folder *folder = NULL;
    if (Obj_Is_A(index, FOLDER)) {
        folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, CHARBUF)) {
        folder = (Folder*)FSFolder_new((CharBuf*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }
    return folder;
}

static void
S_obtain_deletion_lock(PolyReader *self) {
    self->deletion_lock = IxManager_Make_Deletion_Lock(self->manager);
    Lock_Clear_Stale(self->deletion_lock);
    if (!Lock_Obtain(self->deletion_lock)) {
        DECREF(self->deletion_lock);
        self->deletion_lock = NULL;
        THROW(LOCKERR, "Couldn't get commit lock");
    }
}

static void
S_obtain_read_lock(PolyReader *self, const CharBuf *snapshot_file_name) {
    if (!self->manager) { return; }
    self->read_lock = IxManager_Make_Snapshot_Read_Lock(self->manager,
                                                        snapshot_file_name);

    Lock_Clear_Stale(self->read_lock);
    if (!Lock_Obtain(self->read_lock)) {
        DECREF(self->read_lock);
        THROW(LOCKERR, "Couldn't get read lock for %o", snapshot_file_name);
    }
}

static void
S_release_read_lock(PolyReader *self) {
    if (self->read_lock) {
        Lock_Release(self->read_lock);
        DECREF(self->read_lock);
        self->read_lock = NULL;
    }
}

static void
S_release_deletion_lock(PolyReader *self) {
    if (self->deletion_lock) {
        Lock_Release(self->deletion_lock);
        DECREF(self->deletion_lock);
        self->deletion_lock = NULL;
    }
}

int32_t
PolyReader_doc_max(PolyReader *self) {
    return self->doc_max;
}

int32_t
PolyReader_doc_count(PolyReader *self) {
    return self->doc_max - self->del_count;
}

int32_t
PolyReader_del_count(PolyReader *self) {
    return self->del_count;
}

I32Array*
PolyReader_offsets(PolyReader *self) {
    return (I32Array*)INCREF(self->offsets);
}

VArray*
PolyReader_seg_readers(PolyReader *self) {
    return (VArray*)VA_Shallow_Copy(self->sub_readers);
}

VArray*
PolyReader_get_seg_readers(PolyReader *self) {
    return self->sub_readers;
}

uint32_t
PolyReader_sub_tick(I32Array *offsets, int32_t doc_id) {
    int32_t size = I32Arr_Get_Size(offsets);
    if (size == 0) {
        return 0;
    }

    int32_t lo = -1;
    int32_t hi = size;
    while (hi - lo > 1) {
        int32_t mid = lo + ((hi - lo) / 2);
        int32_t offset = I32Arr_Get(offsets, mid);
        if (doc_id <= offset) {
            hi = mid;
        }
        else {
            lo = mid;
        }
    }
    if (hi == size) {
        hi--;
    }

    while (hi > 0) {
        int32_t offset = I32Arr_Get(offsets, hi);
        if (doc_id <= offset) {
            hi--;
        }
        else {
            break;
        }
    }

    return hi;
}


