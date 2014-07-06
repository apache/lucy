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
#include "Lucy/Util/Freezer.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Clownfish/Util/StringHelper.h"

// Obtain/release read locks and commit locks.
static bool
S_obtain_read_lock(PolyReader *self, String *snapshot_filename);
static bool 
S_obtain_deletion_lock(PolyReader *self);
static void
S_release_read_lock(PolyReader *self);
static void
S_release_deletion_lock(PolyReader *self);

// Try to open all SegReaders.
struct try_open_elements_context {
    PolyReader *self;
    VArray     *seg_readers;
};
void
S_try_open_elements(void *context);

// Try to read a Snapshot file.
struct try_read_snapshot_context {
    Snapshot *snapshot;
    Folder   *folder;
    String   *path;
};
static void
S_try_read_snapshot(void *context);

// Try to open an individual SegReader.
struct try_open_segreader_context {
    Schema    *schema;
    Folder    *folder;
    Snapshot  *snapshot;
    VArray    *segments;
    int32_t    seg_tick;
    SegReader *result;
};
static void
S_try_open_segreader(void *context);

static Folder*
S_derive_folder(Obj *index);

PolyReader*
PolyReader_new(Schema *schema, Folder *folder, Snapshot *snapshot,
               IndexManager *manager, VArray *sub_readers) {
    PolyReader *self = (PolyReader*)Class_Make_Obj(POLYREADER);
    return PolyReader_init(self, schema, folder, snapshot, manager,
                           sub_readers);
}

PolyReader*
PolyReader_open(Obj *index, Snapshot *snapshot, IndexManager *manager) {
    PolyReader *self = (PolyReader*)Class_Make_Obj(POLYREADER);
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
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    uint32_t  num_sub_readers = VA_Get_Size(sub_readers);
    int32_t *starts = (int32_t*)MALLOCATE(num_sub_readers * sizeof(int32_t));
    Hash  *data_readers = Hash_new(0);

    DECREF(ivars->sub_readers);
    DECREF(ivars->offsets);
    ivars->sub_readers       = (VArray*)INCREF(sub_readers);

    // Accumulate doc_max, subreader start offsets, and DataReaders.
    ivars->doc_max = 0;
    for (uint32_t i = 0; i < num_sub_readers; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(sub_readers, i);
        Hash *components = SegReader_Get_Components(seg_reader);
        String *api;
        DataReader *component;
        starts[i] = ivars->doc_max;
        ivars->doc_max += SegReader_Doc_Max(seg_reader);
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
    ivars->offsets = I32Arr_new_steal(starts, num_sub_readers);

    String *api;
    VArray *readers;
    Hash_Iterate(data_readers);
    while (Hash_Next(data_readers, (Obj**)&api, (Obj**)&readers)) {
        DataReader *datareader
            = (DataReader*)CERTIFY(S_first_non_null(readers), DATAREADER);
        DataReader *aggregator
            = DataReader_Aggregator(datareader, readers, ivars->offsets);
        if (aggregator) {
            CERTIFY(aggregator, DATAREADER);
            Hash_Store(ivars->components, (Obj*)api, (Obj*)aggregator);
        }
    }
    DECREF(data_readers);

    DeletionsReader *del_reader
        = (DeletionsReader*)Hash_Fetch(
              ivars->components, (Obj*)Class_Get_Name(DELETIONSREADER));
    ivars->del_count = del_reader ? DelReader_Del_Count(del_reader) : 0;
}

PolyReader*
PolyReader_init(PolyReader *self, Schema *schema, Folder *folder,
                Snapshot *snapshot, IndexManager *manager,
                VArray *sub_readers) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    ivars->doc_max    = 0;
    ivars->del_count  = 0;

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
        ivars->sub_readers = VA_new(0);
        ivars->offsets = I32Arr_new_steal(NULL, 0);
    }

    return self;
}

void
PolyReader_Close_IMP(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    PolyReader_Close_t super_close
        = SUPER_METHOD_PTR(POLYREADER, LUCY_PolyReader_Close);
    for (uint32_t i = 0, max = VA_Get_Size(ivars->sub_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(ivars->sub_readers, i);
        SegReader_Close(seg_reader);
    }
    super_close(self);
}

void
PolyReader_Destroy_IMP(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    DECREF(ivars->sub_readers);
    DECREF(ivars->offsets);
    SUPER_DESTROY(self, POLYREADER);
}

static void
S_try_read_snapshot(void *context) {
    struct try_read_snapshot_context *args
        = (struct try_read_snapshot_context*)context;
    Snapshot_Read_File(args->snapshot, args->folder, args->path);
}

static void
S_try_open_segreader(void *context) {
    struct try_open_segreader_context *args
        = (struct try_open_segreader_context*)context;
    args->result = SegReader_new(args->schema, args->folder, args->snapshot,
                                 args->segments, args->seg_tick);
}

void
S_try_open_elements(void *context) {
    struct try_open_elements_context *args
        = (struct try_open_elements_context*)context;
    PolyReader *self              = args->self;
    PolyReaderIVARS *const ivars  = PolyReader_IVARS(self);
    VArray     *files             = Snapshot_List(ivars->snapshot);
    Folder     *folder            = PolyReader_Get_Folder(self);
    uint32_t    num_segs          = 0;
    uint64_t    latest_schema_gen = 0;
    String     *schema_file       = NULL;

    // Find schema file, count segments.
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        String *entry = (String*)VA_Fetch(files, i);

        if (Seg_valid_seg_name(entry)) {
            num_segs++;
        }
        else if (Str_Starts_With_Utf8(entry, "schema_", 7)
                 && Str_Ends_With_Utf8(entry, ".json", 5)
                ) {
            uint64_t gen = IxFileNames_extract_gen(entry);
            if (gen > latest_schema_gen) {
                latest_schema_gen = gen;
                schema_file       = entry;
            }
        }
    }

    // Read Schema.
    if (!schema_file) {
        DECREF(files);
        THROW(ERR, "Can't find a schema file.");
    }
    else {
        Obj *dump = Json_slurp_json(folder, schema_file);
        if (dump) { // read file successfully
            DECREF(ivars->schema);
            ivars->schema = (Schema*)CERTIFY(Freezer_load(dump), SCHEMA);
            DECREF(dump);
            schema_file = NULL;
        }
        else {
            String *mess = MAKE_MESS("Failed to parse %o", schema_file);
            DECREF(files);
            Err_throw_mess(ERR, mess);
        }
    }

    VArray *segments = VA_new(num_segs);
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        String *entry = (String*)VA_Fetch(files, i);

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
                String *mess = MAKE_MESS("Failed to read %o", entry);
                DECREF(segment);
                DECREF(segments);
                DECREF(files);
                Err_throw_mess(ERR, mess);
            }
        }
    }

    // Sort the segments by age.
    VA_Sort(segments, NULL, NULL);

    // Open individual SegReaders.
    struct try_open_segreader_context seg_context;
    seg_context.schema   = PolyReader_Get_Schema(self);
    seg_context.folder   = folder;
    seg_context.snapshot = PolyReader_Get_Snapshot(self);
    seg_context.segments = segments;
    seg_context.result   = NULL;
    args->seg_readers = VA_new(num_segs);
    Err *error = NULL;
    for (uint32_t seg_tick = 0; seg_tick < num_segs; seg_tick++) {
        seg_context.seg_tick = seg_tick;
        error = Err_trap(S_try_open_segreader, &seg_context);
        if (error) {
            break;
        }
        VA_Push(args->seg_readers, (Obj*)seg_context.result);
        seg_context.result = NULL;
    }

    DECREF(segments);
    DECREF(files);
    if (error) {
        DECREF(args->seg_readers);
        args->seg_readers = NULL;
        RETHROW(error);
    }
}

// For test suite.
String* PolyReader_race_condition_debug1 = NULL;
int32_t  PolyReader_debug1_num_passes     = 0;

PolyReader*
PolyReader_do_open(PolyReader *self, Obj *index, Snapshot *snapshot,
                   IndexManager *manager) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    Folder   *folder   = S_derive_folder(index);
    uint64_t  last_gen = 0;

    PolyReader_init(self, NULL, folder, snapshot, manager, NULL);
    DECREF(folder);

    if (manager) { 
        if (!S_obtain_deletion_lock(self)) {
            DECREF(self);
            THROW(LOCKERR, "Couldn't get deletion lock");
        }
    }

    while (1) {
        String *target_snap_file;

        // If a Snapshot was supplied, use its file.
        if (snapshot) {
            target_snap_file = Snapshot_Get_Path(snapshot);
            if (!target_snap_file) {
                THROW(ERR, "Supplied snapshot objects must not be empty");
            }
            else {
                target_snap_file = (String*)INCREF(target_snap_file);
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
            if (!S_obtain_read_lock(self, target_snap_file)) {
                DECREF(self);
                THROW(LOCKERR, "Couldn't get read lock for %o",
                      target_snap_file);
            }
        }

        // Testing only.
        if (PolyReader_race_condition_debug1) {
            StackString *temp = SSTR_WRAP_UTF8("temp", 4);
            if (Folder_Exists(folder, (String*)temp)) {
                bool success = Folder_Rename(folder, (String*)temp,
                                               PolyReader_race_condition_debug1);
                if (!success) { RETHROW(INCREF(Err_get_error())); }
            }
            PolyReader_debug1_num_passes++;
        }

        // If a Snapshot object was passed in, the file has already been read.
        // If that's not the case, we must read the file we just picked.
        if (!snapshot) {
            struct try_read_snapshot_context context;
            context.snapshot = ivars->snapshot;
            context.folder   = folder;
            context.path     = target_snap_file;
            Err *error = Err_trap(S_try_read_snapshot, &context);

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
                    RETHROW(error);
                }
            }
        }

        /* It's possible, though unlikely, for an Indexer to delete files
         * out from underneath us after the snapshot file is read but before
         * we've got SegReaders holding open all the required files.  If we
         * failed to open something, see if we can find a newer snapshot file.
         * If we can, then the exception was due to the race condition.  If
         * not, we have a real exception, so throw an error. */
        struct try_open_elements_context context;
        context.self        = self;
        context.seg_readers = NULL;
        Err *error = Err_trap(S_try_open_elements, &context);
        if (error) {
            S_release_read_lock(self);
            DECREF(target_snap_file);
            if (last_gen < gen) { // Index updated, so try again.
                DECREF(error);
                last_gen = gen;
            }
            else { // Real error.
                if (manager) { S_release_deletion_lock(self); }
                RETHROW(error);
            }
        }
        else { // Succeeded.
            S_init_sub_readers(self, (VArray*)context.seg_readers);
            DECREF(context.seg_readers);
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
    else if (Obj_Is_A(index, STRING)) {
        folder = (Folder*)FSFolder_new((String*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }
    return folder;
}

static bool 
S_obtain_deletion_lock(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    ivars->deletion_lock = IxManager_Make_Deletion_Lock(ivars->manager);
    Lock_Clear_Stale(ivars->deletion_lock);
    if (!Lock_Obtain(ivars->deletion_lock)) {
        DECREF(ivars->deletion_lock);
        ivars->deletion_lock = NULL;
        return false;
    }
    return true;
}

static bool
S_obtain_read_lock(PolyReader *self, String *snapshot_file_name) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    ivars->read_lock = IxManager_Make_Snapshot_Read_Lock(ivars->manager,
                                                         snapshot_file_name);

    Lock_Clear_Stale(ivars->read_lock);
    if (!Lock_Obtain(ivars->read_lock)) {
        DECREF(ivars->read_lock);
        ivars->read_lock = NULL;
        return false;
    }
    return true;
}

static void
S_release_read_lock(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    if (ivars->read_lock) {
        Lock_Release(ivars->read_lock);
        DECREF(ivars->read_lock);
        ivars->read_lock = NULL;
    }
}

static void
S_release_deletion_lock(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    if (ivars->deletion_lock) {
        Lock_Release(ivars->deletion_lock);
        DECREF(ivars->deletion_lock);
        ivars->deletion_lock = NULL;
    }
}

int32_t
PolyReader_Doc_Max_IMP(PolyReader *self) {
    return PolyReader_IVARS(self)->doc_max;
}

int32_t
PolyReader_Doc_Count_IMP(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    return ivars->doc_max - ivars->del_count;
}

int32_t
PolyReader_Del_Count_IMP(PolyReader *self) {
    return PolyReader_IVARS(self)->del_count;
}

I32Array*
PolyReader_Offsets_IMP(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    return (I32Array*)INCREF(ivars->offsets);
}

VArray*
PolyReader_Seg_Readers_IMP(PolyReader *self) {
    PolyReaderIVARS *const ivars = PolyReader_IVARS(self);
    return (VArray*)VA_Shallow_Copy(ivars->sub_readers);
}

VArray*
PolyReader_Get_Seg_Readers_IMP(PolyReader *self) {
    return PolyReader_IVARS(self)->sub_readers;
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


