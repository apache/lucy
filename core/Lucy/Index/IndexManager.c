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

#include <stdlib.h>

static const int32_t S_fibonacci[47] = {
    0,
    1,
    1,
    2,
    3,
    5,
    8,
    13,
    21,
    34,
    55,
    89,
    144,
    233,
    377,
    610,
    987,
    1597,
    2584,
    4181,
    6765,
    10946,
    17711,
    28657,
    46368,
    75025,
    121393,
    196418,
    317811,
    514229,
    832040,
    1346269,
    2178309,
    3524578,
    5702887,
    9227465,
    14930352,
    24157817,
    39088169,
    63245986,
    102334155,
    165580141,
    267914296,
    433494437,
    701408733,
    1134903170,
    1836311903
};

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
    Vector *files = Snapshot_List(snapshot);
    uint64_t highest_seg_num = 0;
    UNUSED_VAR(self);
    for (size_t i = 0, max = Vec_Get_Size(files); i < max; i++) {
        String *file = (String*)Vec_Fetch(files, i);
        if (Seg_valid_seg_name(file)) {
            uint64_t seg_num = IxFileNames_extract_gen(file);
            if (seg_num > highest_seg_num) { highest_seg_num = seg_num; }
        }
    }
    DECREF(files);
    return (int64_t)highest_seg_num;
}

static int
S_compare_doc_count(const void *va, const void *vb) {
    SegReader *a = *(SegReader**)va;
    SegReader *b = *(SegReader**)vb;
    return SegReader_Doc_Count(a) - SegReader_Doc_Count(b);
}

Vector*
IxManager_Recycle_IMP(IndexManager *self, PolyReader *reader,
                      DeletionsWriter *del_writer, int64_t cutoff,
                      bool optimize) {
    Vector *seg_readers = PolyReader_Get_Seg_Readers(reader);
    size_t num_seg_readers = Vec_Get_Size(seg_readers);
    SegReader **candidates
        = (SegReader**)MALLOCATE(num_seg_readers * sizeof(SegReader*));
    size_t num_candidates = 0;
    for (size_t i = 0; i < num_seg_readers; i++) {
        SegReader *seg_reader = (SegReader*)Vec_Fetch(seg_readers, i);
        if (SegReader_Get_Seg_Num(seg_reader) > cutoff) {
            candidates[num_candidates++] = seg_reader;
        }
    }

    Vector *recyclables = Vec_new(num_candidates);

    if (optimize) {
        for (size_t i = 0; i < num_candidates; i++) {
            Vec_Push(recyclables, INCREF(candidates[i]));
        }
        FREEMEM(candidates);
        return recyclables;
    }

    // Sort by ascending size in docs, choose sparsely populated segments.
    qsort(candidates, num_candidates, sizeof(SegReader*), S_compare_doc_count);
    int32_t *counts = (int32_t*)MALLOCATE(num_candidates * sizeof(int32_t));
    for (uint32_t i = 0; i < num_candidates; i++) {
        counts[i] = SegReader_Doc_Count(candidates[i]);
    }
    I32Array *doc_counts = I32Arr_new_steal(counts, num_candidates);
    uint32_t threshold = IxManager_Choose_Sparse(self, doc_counts);
    DECREF(doc_counts);

    // Move SegReaders to be recycled.
    for (uint32_t i = 0; i < threshold; i++) {
        Vec_Store(recyclables, i, INCREF(candidates[i]));
    }

    // Find segments where at least 10% of all docs have been deleted.
    for (uint32_t i = threshold; i < num_candidates; i++) {
        SegReader *seg_reader = candidates[i];
        String    *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        double doc_max = SegReader_Doc_Max(seg_reader);
        double num_deletions = DelWriter_Seg_Del_Count(del_writer, seg_name);
        double del_proportion = num_deletions / doc_max;
        if (del_proportion >= 0.1) {
            Vec_Push(recyclables, INCREF(seg_reader));
        }
    }

    FREEMEM(candidates);
    return recyclables;
}

uint32_t
IxManager_Choose_Sparse_IMP(IndexManager *self, I32Array *doc_counts) {
    UNUSED_VAR(self);
    uint32_t threshold  = 0;
    int32_t total_docs = 0;
    const uint32_t num_candidates = (uint32_t)I32Arr_Get_Size(doc_counts);

    // Find sparsely populated segments.
    for (uint32_t i = 0; i < num_candidates; i++) {
        uint32_t num_segs_when_done = num_candidates - threshold + 1;
        total_docs += I32Arr_Get(doc_counts, i);
        uint32_t n = num_segs_when_done + 5;
        if (n >= sizeof(S_fibonacci) / sizeof(S_fibonacci[0])
            || total_docs < S_fibonacci[n]) {
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
    String *write_lock_name = SSTR_WRAP_C("write");
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, write_lock_name,
                              (int32_t)ivars->write_lock_timeout,
                              (int32_t)ivars->write_lock_interval,
                              true);
}

Lock*
IxManager_Make_Deletion_Lock_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    String *lock_name = SSTR_WRAP_C("deletion");
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, lock_name,
                              (int32_t)ivars->deletion_lock_timeout,
                              (int32_t)ivars->deletion_lock_interval,
                              true);
}

Lock*
IxManager_Make_Merge_Lock_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    String *merge_lock_name = SSTR_WRAP_C("merge");
    LockFactory *lock_factory = S_obtain_lock_factory(self);
    return LockFact_Make_Lock(lock_factory, merge_lock_name,
                              (int32_t)ivars->merge_lock_timeout,
                              (int32_t)ivars->merge_lock_interval,
                              true);
}

void
IxManager_Write_Merge_Data_IMP(IndexManager *self, int64_t cutoff) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    String *merge_json = SSTR_WRAP_C("merge.json");
    Hash *data = Hash_new(1);
    bool success;
    Hash_Store_Utf8(data, "cutoff", 6, (Obj*)Str_newf("%i64", cutoff));
    success = Json_spew_json((Obj*)data, ivars->folder, merge_json);
    DECREF(data);
    if (!success) {
        THROW(ERR, "Failed to write to %o", merge_json);
    }
}

Hash*
IxManager_Read_Merge_Data_IMP(IndexManager *self) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    String *merge_json = SSTR_WRAP_C("merge.json");
    if (Folder_Exists(ivars->folder, merge_json)) {
        Hash *stuff = (Hash*)Json_slurp_json(ivars->folder, merge_json);
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
    String *merge_json = SSTR_WRAP_C("merge.json");
    return Folder_Delete(ivars->folder, merge_json) != 0;
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

    Lock *lock = LockFact_Make_Lock(lock_factory, lock_name, 1000, 100, false);

    DECREF(lock_name);
    return lock;
}

void
IxManager_Set_Folder_IMP(IndexManager *self, Folder *folder) {
    IndexManagerIVARS *const ivars = IxManager_IVARS(self);
    Folder *temp = ivars->folder;
    ivars->folder = (Folder*)INCREF(folder);
    DECREF(temp);
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
    if (timeout > INT32_MAX) {
        THROW(ERR, "Timeout can't be greater than INT32_MAX: %u32", timeout);
    }
    IxManager_IVARS(self)->write_lock_timeout = timeout;
}

void
IxManager_Set_Write_Lock_Interval_IMP(IndexManager *self, uint32_t interval) {
    if (interval > INT32_MAX) {
        THROW(ERR, "Interval can't be greater than INT32_MAX: %u32", interval);
    }
    IxManager_IVARS(self)->write_lock_interval = interval;
}

void
IxManager_Set_Merge_Lock_Timeout_IMP(IndexManager *self, uint32_t timeout) {
    if (timeout > INT32_MAX) {
        THROW(ERR, "Timeout can't be greater than INT32_MAX: %u32", timeout);
    }
    IxManager_IVARS(self)->merge_lock_timeout = timeout;
}

void
IxManager_Set_Merge_Lock_Interval_IMP(IndexManager *self, uint32_t interval) {
    if (interval > INT32_MAX) {
        THROW(ERR, "Interval can't be greater than INT32_MAX: %u32", interval);
    }
    IxManager_IVARS(self)->merge_lock_interval = interval;
}

void
IxManager_Set_Deletion_Lock_Timeout_IMP(IndexManager *self,
                                        uint32_t timeout) {
    if (timeout > INT32_MAX) {
        THROW(ERR, "Timeout can't be greater than INT32_MAX: %u32", timeout);
    }
    IxManager_IVARS(self)->deletion_lock_timeout = timeout;
}

void
IxManager_Set_Deletion_Lock_Interval_IMP(IndexManager *self,
                                         uint32_t interval) {
    if (interval > INT32_MAX) {
        THROW(ERR, "Interval can't be greater than INT32_MAX: %u32", interval);
    }
    IxManager_IVARS(self)->deletion_lock_interval = interval;
}


