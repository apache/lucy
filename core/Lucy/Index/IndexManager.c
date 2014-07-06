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

#define C_LUCY_INDEXMANAGER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Store/LockFactory.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"
#include "Clownfish/Util/StringHelper.h"

IndexManager*
IxManager_new(String *host, LockFactory *lock_factory) {
    IndexManager *self = (IndexManager*)Class_Make_Obj(INDEXMANAGER);
    return IxManager_init(self, host, lock_factory);
}

IndexManager*
IxManager_init(IndexManager *self, String *host,
               LockFactory *lock_factory) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    ivars->host                = host
                                ? Str_Clone(host)
                                : Str_new_from_trusted_utf8("", 0);
    ivars->lock_factory        = (LockFactory*)INCREF(lock_factory);
    ivars->folder              = NULL;
    ivars->write_lock_timeout  = 1000;
    ivars->write_lock_interval = 100;
    ivars->merge_lock_timeout  = 0;
    ivars->merge_lock_interval = 1000;
    ivars->deletion_lock_timeout  = 1000;
    ivars->deletion_lock_interval = 100;

    return self;
}

void
IxManager_Destroy_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    DECREF(ivars->host);
    DECREF(ivars->folder);
    DECREF(ivars->lock_factory);
    SUPER_DESTROY(self, INDEXMANAGER);
}

int64_t
IxManager_Highest_Seg_Num_IMP(IndexManager *self, Snapshot *snapshot) {
    VArray *files = Snapshot_List(snapshot);
    uint64_t highest_seg_num = 0;
    UNUSED_VAR(self);
    for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
        String *file = (String*)VA_Fetch(files, i);
        if (Seg_valid_seg_name(file)) {
            uint64_t seg_num = IxFileNames_extract_gen(file);
            if (seg_num > highest_seg_num) { highest_seg_num = seg_num; }
        }
    }
    DECREF(files);
    return (int64_t)highest_seg_num;
}

String*
IxManager_Make_Snapshot_Filename_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    Folder *folder = (Folder*)CERTIFY(ivars->folder, FOLDER);
    DirHandle *dh = Folder_Open_Dir(folder, NULL);
    uint64_t max_gen = 0;

    if (!dh) { RETHROW(INCREF(Err_get_error())); }
    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (Str_Starts_With_Utf8(entry, "snapshot_", 9)
            && Str_Ends_With_Utf8(entry, ".json", 5)
           ) {
            uint64_t gen = IxFileNames_extract_gen(entry);
            if (gen > max_gen) { max_gen = gen; }
        }
        DECREF(entry);
    }
    DECREF(dh);

    uint64_t new_gen = max_gen + 1;
    char  base36[StrHelp_MAX_BASE36_BYTES];
    StrHelp_to_base36(new_gen, &base36);
    return Str_newf("snapshot_%s.json", &base36);
}

static int
S_compare_doc_count(void *context, const void *va, const void *vb) {
    SegReader *a = *(SegReader**)va;
    SegReader *b = *(SegReader**)vb;
    UNUSED_VAR(context);
    return SegReader_Doc_Count(a) - SegReader_Doc_Count(b);
}

static bool
S_check_cutoff(VArray *array, uint32_t tick, void *data) {
    SegReader *seg_reader = (SegReader*)VA_Fetch(array, tick);
    int64_t cutoff = *(int64_t*)data;
    return SegReader_Get_Seg_Num(seg_reader) > cutoff;
}

static uint32_t
S_fibonacci(uint32_t n) {
    uint32_t result = 0;
    if (n > 46) {
        THROW(ERR, "input %u32 too high", n);
    }
    else if (n < 2) {
        result = n;
    }
    else {
        result = S_fibonacci(n - 1) + S_fibonacci(n - 2);
    }
    return result;
}

VArray*
IxManager_Recycle_IMP(IndexManager *self, PolyReader *reader,
                      DeletionsWriter *del_writer, int64_t cutoff,
                      bool optimize) {
    VArray *seg_readers = PolyReader_Get_Seg_Readers(reader);
    VArray *candidates  = VA_Gather(seg_readers, S_check_cutoff, &cutoff);
    VArray *recyclables = VA_new(VA_Get_Size(candidates));
    const uint32_t num_candidates = VA_Get_Size(candidates);

    if (optimize) {
        DECREF(recyclables);
        return candidates;
    }

    // Sort by ascending size in docs, choose sparsely populated segments.
    VA_Sort(candidates, S_compare_doc_count, NULL);
    int32_t *counts = (int32_t*)MALLOCATE(num_candidates * sizeof(int32_t));
    for (uint32_t i = 0; i < num_candidates; i++) {
        SegReader *seg_reader
            = (SegReader*)CERTIFY(VA_Fetch(candidates, i), SEGREADER);
        counts[i] = SegReader_Doc_Count(seg_reader);
    }
    I32Array *doc_counts = I32Arr_new_steal(counts, num_candidates);
    uint32_t threshold = IxManager_Choose_Sparse(self, doc_counts);
    DECREF(doc_counts);

    // Move SegReaders to be recycled.
    for (uint32_t i = 0; i < threshold; i++) {
        VA_Store(recyclables, i, VA_Delete(candidates, i));
    }

    // Find segments where at least 10% of all docs have been deleted.
    for (uint32_t i = threshold; i < num_candidates; i++) {
        SegReader *seg_reader = (SegReader*)VA_Delete(candidates, i);
        String    *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        double doc_max = SegReader_Doc_Max(seg_reader);
        double num_deletions = DelWriter_Seg_Del_Count(del_writer, seg_name);
        double del_proportion = num_deletions / doc_max;
        if (del_proportion >= 0.1) {
            VA_Push(recyclables, (Obj*)seg_reader);
        }
        else {
            DECREF(seg_reader);
        }
    }

    DECREF(candidates);
    return recyclables;
}

uint32_t
IxManager_Choose_Sparse_IMP(IndexManager *self, I32Array *doc_counts) {
    UNUSED_VAR(self);
    uint32_t threshold  = 0;
    uint32_t total_docs = 0;
    const uint32_t num_candidates = I32Arr_Get_Size(doc_counts);

    // Find sparsely populated segments.
    for (uint32_t i = 0; i < num_candidates; i++) {
        uint32_t num_segs_when_done = num_candidates - threshold + 1;
        total_docs += I32Arr_Get(doc_counts, i);
        if (total_docs < S_fibonacci(num_segs_when_done + 5)) {
            threshold = i + 1;
        }
    }

    // If recycling, try not to get stuck merging the same big segment over
    // and over on small commits.
    if (threshold == 1 && num_candidates > 2) {
        int32_t this_seg_doc_count = I32Arr_Get(doc_counts, 0);
        int32_t next_seg_doc_count = I32Arr_Get(doc_counts, 1);
        // Try to merge 2 segments worth of stuff, so long as the next segment
        // is less than double the size.
        if (next_seg_doc_count / 2 < this_seg_doc_count) {
            threshold = 2;
        }
    }

    return threshold;
}

static LockFactory*
S_obtain_lock_factory(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    if (!ivars->lock_factory) {
        if (!ivars->folder) {
            THROW(ERR, "Can't create a LockFactory without a Folder");
        }
        ivars->lock_factory = LockFact_new(ivars->folder, ivars->host);
    }
    return ivars->lock_factory;
}

Lock*
IxManager_Make_Write_Lock_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *write_lock_name = SSTR_WRAP_UTF8("write", 5);
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, (String*)write_lock_name,
                              ivars->write_lock_timeout,
                              ivars->write_lock_interval);
}

Lock*
IxManager_Make_Deletion_Lock_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *lock_name = SSTR_WRAP_UTF8("deletion", 8);
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, (String*)lock_name,
                              ivars->deletion_lock_timeout,
                              ivars->deletion_lock_interval);
}

Lock*
IxManager_Make_Merge_Lock_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *merge_lock_name = SSTR_WRAP_UTF8("merge", 5);
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, (String*)merge_lock_name,
                              ivars->merge_lock_timeout,
                              ivars->merge_lock_interval);
}

void
IxManager_Write_Merge_Data_IMP(IndexManager *self, int64_t cutoff) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *merge_json = SSTR_WRAP_UTF8("merge.json", 10);
    Hash *data = Hash_new(1);
    bool success;
    Hash_Store_Utf8(data, "cutoff", 6, (Obj*)Str_newf("%i64", cutoff));
    success = Json_spew_json((Obj*)data, ivars->folder, (String*)merge_json);
    DECREF(data);
    if (!success) {
        THROW(ERR, "Failed to write to %o", merge_json);
    }
}

Hash*
IxManager_Read_Merge_Data_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *merge_json = SSTR_WRAP_UTF8("merge.json", 10);
    if (Folder_Exists(ivars->folder, (String*)merge_json)) {
        Hash *stuff
            = (Hash*)Json_slurp_json(ivars->folder, (String*)merge_json);
        if (stuff) {
            CERTIFY(stuff, HASH);
            return stuff;
        }
        else {
            return Hash_new(0);
        }
    }
    else {
        return NULL;
    }
}

bool
IxManager_Remove_Merge_Data_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    StackString *merge_json = SSTR_WRAP_UTF8("merge.json", 10);
    return Folder_Delete(ivars->folder, (String*)merge_json) != 0;
}

Lock*
IxManager_Make_Snapshot_Read_Lock_IMP(IndexManager *self,
                                      String *filename) {
    LockFactory *lock_factory = S_obtain_lock_factory(self);

    if (!Str_Starts_With_Utf8(filename, "snapshot_", 9)
        || !Str_Ends_With_Utf8(filename, ".json", 5)
       ) {
        THROW(ERR, "Not a snapshot filename: %o", filename);
    }

    // Truncate ".json" from end of snapshot file name.
    size_t lock_name_len = Str_Length(filename) - (sizeof(".json") - 1);
    String *lock_name = Str_SubString(filename, 0, lock_name_len);

    Lock *lock = LockFact_Make_Shared_Lock(lock_factory, lock_name, 1000, 100);

    DECREF(lock_name);
    return lock;
}

void
IxManager_Set_Folder_IMP(IndexManager *self, Folder *folder) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    DECREF(ivars->folder);
    ivars->folder = (Folder*)INCREF(folder);
}

Folder*
IxManager_Get_Folder_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->folder;
}

String*
IxManager_Get_Host_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->host;
}

uint32_t
IxManager_Get_Write_Lock_Timeout_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->write_lock_timeout;
}

uint32_t
IxManager_Get_Write_Lock_Interval_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->write_lock_interval;
}

uint32_t
IxManager_Get_Merge_Lock_Timeout_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->merge_lock_timeout;
}

uint32_t
IxManager_Get_Merge_Lock_Interval_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->merge_lock_interval;
}

uint32_t
IxManager_Get_Deletion_Lock_Timeout_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->deletion_lock_timeout;
}

uint32_t
IxManager_Get_Deletion_Lock_Interval_IMP(IndexManager *self) {
    return IxManager_IVARS(self)->deletion_lock_interval;
}

void
IxManager_Set_Write_Lock_Timeout_IMP(IndexManager *self, uint32_t timeout) {
    IxManager_IVARS(self)->write_lock_timeout = timeout;
}

void
IxManager_Set_Write_Lock_Interval_IMP(IndexManager *self, uint32_t interval) {
    IxManager_IVARS(self)->write_lock_interval = interval;
}

void
IxManager_Set_Merge_Lock_Timeout_IMP(IndexManager *self, uint32_t timeout) {
    IxManager_IVARS(self)->merge_lock_timeout = timeout;
}

void
IxManager_Set_Merge_Lock_Interval_IMP(IndexManager *self, uint32_t interval) {
    IxManager_IVARS(self)->merge_lock_interval = interval;
}

void
IxManager_Set_Deletion_Lock_Timeout_IMP(IndexManager *self,
                                        uint32_t timeout) {
    IxManager_IVARS(self)->deletion_lock_timeout = timeout;
}

void
IxManager_Set_Deletion_Lock_Interval_IMP(IndexManager *self,
                                         uint32_t interval) {
    IxManager_IVARS(self)->deletion_lock_interval = interval;
}


