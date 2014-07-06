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

#define C_LUCY_BACKGROUNDMERGER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/BackgroundMerger.h"
#include "Lucy/Index/DeletionsReader.h"
#include "Lucy/Index/DeletionsWriter.h"
#include "Lucy/Index/FilePurger.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/PolyReader.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/SegReader.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Index/SegWriter.h"
#include "Lucy/Plan/Architecture.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Search/Matcher.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Util/Freezer.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

// Verify a Folder or derive an FSFolder from a String path.
static Folder*
S_init_folder(Obj *index);

// Grab the write lock and store it in self.
static void
S_obtain_write_lock(BackgroundMerger *self);

// Grab the merge lock and store it in self.
static void
S_obtain_merge_lock(BackgroundMerger *self);

// Release the write lock - if it's there.
static void
S_release_write_lock(BackgroundMerger *self);

// Release the merge lock - if it's there.
static void
S_release_merge_lock(BackgroundMerger *self);

BackgroundMerger*
BGMerger_new(Obj *index, IndexManager *manager) {
    BackgroundMerger *self
        = (BackgroundMerger*)Class_Make_Obj(BACKGROUNDMERGER);
    return BGMerger_init(self, index, manager);
}

BackgroundMerger*
BGMerger_init(BackgroundMerger *self, Obj *index, IndexManager *manager) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    Folder *folder = S_init_folder(index);

    // Init.
    ivars->optimize      = false;
    ivars->prepared      = false;
    ivars->needs_commit  = false;
    ivars->snapfile      = NULL;
    ivars->doc_maps      = Hash_new(0);

    // Assign.
    ivars->folder = folder;
    if (manager) {
        ivars->manager = (IndexManager*)INCREF(manager);
    }
    else {
        ivars->manager = IxManager_new(NULL, NULL);
        IxManager_Set_Write_Lock_Timeout(ivars->manager, 10000);
    }
    IxManager_Set_Folder(ivars->manager, folder);

    // Obtain write lock (which we'll only hold briefly), then merge lock.
    S_obtain_write_lock(self);
    if (!ivars->write_lock) {
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }
    S_obtain_merge_lock(self);
    if (!ivars->merge_lock) {
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }

    // Find the latest snapshot.  If there's no index content, bail early.
    ivars->snapshot = Snapshot_Read_File(Snapshot_new(), folder, NULL);
    if (!Snapshot_Get_Path(ivars->snapshot)) {
        S_release_write_lock(self);
        S_release_merge_lock(self);
        return self;
    }

    // Create FilePurger. Zap detritus from previous sessions.
    ivars->file_purger = FilePurger_new(folder, ivars->snapshot, ivars->manager);
    FilePurger_Purge(ivars->file_purger);

    // Open a PolyReader, passing in the IndexManager so we get a read lock on
    // the Snapshot's files -- so that Indexers don't zap our files while
    // we're operating in the background.
    ivars->polyreader = PolyReader_open((Obj*)folder, NULL, ivars->manager);

    // Clone the PolyReader's schema.
    Obj *dump = (Obj*)Schema_Dump(PolyReader_Get_Schema(ivars->polyreader));
    ivars->schema = (Schema*)CERTIFY(Freezer_load(dump), SCHEMA);
    DECREF(dump);

    // Create new Segment.
    int64_t new_seg_num
        = IxManager_Highest_Seg_Num(ivars->manager, ivars->snapshot) + 1;
    VArray *fields = Schema_All_Fields(ivars->schema);
    ivars->segment = Seg_new(new_seg_num);
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        Seg_Add_Field(ivars->segment, (String*)VA_Fetch(fields, i));
    }
    DECREF(fields);

    // Our "cutoff" is the segment this BackgroundMerger will write.  Now that
    // we've determined the cutoff, write the merge data file.
    ivars->cutoff = Seg_Get_Number(ivars->segment);
    IxManager_Write_Merge_Data(ivars->manager, ivars->cutoff);

    /* Create the SegWriter but hold off on preparing the new segment
     * directory -- because if we don't need to merge any segments we don't
     * need it.  (We've reserved the dir by plopping down the merge.json
     * file.) */
    ivars->seg_writer = SegWriter_new(ivars->schema, ivars->snapshot,
                                      ivars->segment, ivars->polyreader);

    // Grab a local ref to the DeletionsWriter.
    ivars->del_writer
        = (DeletionsWriter*)INCREF(SegWriter_Get_Del_Writer(ivars->seg_writer));

    // Release the write lock.  Now new Indexers can start while we work in
    // the background.
    S_release_write_lock(self);

    return self;
}

void
BGMerger_Destroy_IMP(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    S_release_merge_lock(self);
    S_release_write_lock(self);
    DECREF(ivars->schema);
    DECREF(ivars->folder);
    DECREF(ivars->segment);
    DECREF(ivars->manager);
    DECREF(ivars->polyreader);
    DECREF(ivars->del_writer);
    DECREF(ivars->snapshot);
    DECREF(ivars->seg_writer);
    DECREF(ivars->file_purger);
    DECREF(ivars->write_lock);
    DECREF(ivars->snapfile);
    DECREF(ivars->doc_maps);
    SUPER_DESTROY(self, BACKGROUNDMERGER);
}

static Folder*
S_init_folder(Obj *index) {
    Folder *folder = NULL;

    // Validate or acquire a Folder.
    if (Obj_Is_A(index, FOLDER)) {
        folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, STRING)) {
        folder = (Folder*)FSFolder_new((String*)index);
    }
    else {
        THROW(ERR, "Invalid type for 'index': %o", Obj_Get_Class_Name(index));
    }

    // Validate index directory.
    if (!Folder_Check(folder)) {
        THROW(ERR, "Folder '%o' failed check", Folder_Get_Path(folder));
    }

    return folder;
}

void
BGMerger_Optimize_IMP(BackgroundMerger *self) {
    BGMerger_IVARS(self)->optimize = true;
}

static uint32_t
S_maybe_merge(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    VArray *to_merge = IxManager_Recycle(ivars->manager, ivars->polyreader,
                                         ivars->del_writer, 0, ivars->optimize);
    int32_t num_to_merge = VA_Get_Size(to_merge);

    // There's no point in merging one segment if it has no deletions, because
    // we'd just be rewriting it. */
    if (num_to_merge == 1) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(to_merge, 0);
        if (!SegReader_Del_Count(seg_reader)) {
            DECREF(to_merge);
            return 0;
        }
    }
    else if (num_to_merge == 0) {
        DECREF(to_merge);
        return 0;
    }

    // Now that we're sure we're writing a new segment, prep the seg dir.
    SegWriter_Prep_Seg_Dir(ivars->seg_writer);

    // Consolidate segments.
    for (uint32_t i = 0, max = num_to_merge; i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(to_merge, i);
        String    *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        int64_t    doc_count  = Seg_Get_Count(ivars->segment);
        Matcher *deletions
            = DelWriter_Seg_Deletions(ivars->del_writer, seg_reader);
        I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                ivars->del_writer, deletions,
                                SegReader_Doc_Max(seg_reader),
                                (int32_t)doc_count);

        Hash_Store(ivars->doc_maps, (Obj*)seg_name, (Obj*)doc_map);
        SegWriter_Merge_Segment(ivars->seg_writer, seg_reader, doc_map);
        DECREF(deletions);
    }

    DECREF(to_merge);
    return num_to_merge;
}

static bool
S_merge_updated_deletions(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    Hash *updated_deletions = NULL;

    PolyReader *new_polyreader
        = PolyReader_open((Obj*)ivars->folder, NULL, NULL);
    VArray *new_seg_readers
        = PolyReader_Get_Seg_Readers(new_polyreader);
    VArray *old_seg_readers
        = PolyReader_Get_Seg_Readers(ivars->polyreader);
    Hash *new_segs = Hash_new(VA_Get_Size(new_seg_readers));

    for (uint32_t i = 0, max = VA_Get_Size(new_seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(new_seg_readers, i);
        String    *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        Hash_Store(new_segs, (Obj*)seg_name, INCREF(seg_reader));
    }

    for (uint32_t i = 0, max = VA_Get_Size(old_seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(old_seg_readers, i);
        String    *seg_name   = SegReader_Get_Seg_Name(seg_reader);

        // If this segment was merged away...
        if (Hash_Fetch(ivars->doc_maps, (Obj*)seg_name)) {
            SegReader *new_seg_reader
                = (SegReader*)CERTIFY(
                      Hash_Fetch(new_segs, (Obj*)seg_name),
                      SEGREADER);
            int32_t old_del_count = SegReader_Del_Count(seg_reader);
            int32_t new_del_count = SegReader_Del_Count(new_seg_reader);
            // ... were any new deletions applied against it?
            if (old_del_count != new_del_count) {
                DeletionsReader *del_reader
                    = (DeletionsReader*)SegReader_Obtain(
                          new_seg_reader,
                          Class_Get_Name(DELETIONSREADER));
                if (!updated_deletions) {
                    updated_deletions = Hash_new(max);
                }
                Hash_Store(updated_deletions, (Obj*)seg_name,
                           (Obj*)DelReader_Iterator(del_reader));
            }
        }
    }

    DECREF(new_polyreader);
    DECREF(new_segs);

    if (!updated_deletions) {
        return false;
    }
    else {
        PolyReader *merge_polyreader
            = PolyReader_open((Obj*)ivars->folder, ivars->snapshot, NULL);
        VArray *merge_seg_readers
            = PolyReader_Get_Seg_Readers(merge_polyreader);
        Snapshot *latest_snapshot
            = Snapshot_Read_File(Snapshot_new(), ivars->folder, NULL);
        int64_t new_seg_num
            = IxManager_Highest_Seg_Num(ivars->manager, latest_snapshot) + 1;
        Segment   *new_segment = Seg_new(new_seg_num);
        SegWriter *seg_writer  = SegWriter_new(ivars->schema, ivars->snapshot,
                                               new_segment, merge_polyreader);
        DeletionsWriter *del_writer = SegWriter_Get_Del_Writer(seg_writer);
        int64_t  merge_seg_num = Seg_Get_Number(ivars->segment);
        uint32_t seg_tick      = INT32_MAX;
        int32_t  offset        = INT32_MAX;
        String  *seg_name      = NULL;
        Matcher *deletions     = NULL;

        SegWriter_Prep_Seg_Dir(seg_writer);

        for (uint32_t i = 0, max = VA_Get_Size(merge_seg_readers); i < max; i++) {
            SegReader *seg_reader
                = (SegReader*)VA_Fetch(merge_seg_readers, i);
            if (SegReader_Get_Seg_Num(seg_reader) == merge_seg_num) {
                I32Array *offsets = PolyReader_Offsets(merge_polyreader);
                seg_tick = i;
                offset = I32Arr_Get(offsets, seg_tick);
                DECREF(offsets);
            }
        }
        if (offset == INT32_MAX) { THROW(ERR, "Failed sanity check"); }

        Hash_Iterate(updated_deletions);
        while (Hash_Next(updated_deletions,
                         (Obj**)&seg_name, (Obj**)&deletions)
              ) {
            I32Array *doc_map
                = (I32Array*)CERTIFY(
                      Hash_Fetch(ivars->doc_maps, (Obj*)seg_name),
                      I32ARRAY);
            int32_t del;
            while (0 != (del = Matcher_Next(deletions))) {
                // Find the slot where the deleted doc resides in the
                // rewritten segment. If the doc was already deleted when we
                // were merging, do nothing.
                int32_t remapped = I32Arr_Get(doc_map, del);
                if (remapped) {
                    // It's a new deletion, so carry it forward and zap it in
                    // the rewritten segment.
                    DelWriter_Delete_By_Doc_ID(del_writer, remapped + offset);
                }
            }
        }

        // Finish the segment and clean up.
        DelWriter_Finish(del_writer);
        SegWriter_Finish(seg_writer);
        DECREF(seg_writer);
        DECREF(new_segment);
        DECREF(latest_snapshot);
        DECREF(merge_polyreader);
        DECREF(updated_deletions);
    }

    return true;
}

void
BGMerger_Prepare_Commit_IMP(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    VArray   *seg_readers     = PolyReader_Get_Seg_Readers(ivars->polyreader);
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    uint32_t  segs_merged     = 0;

    if (ivars->prepared) {
        THROW(ERR, "Can't call Prepare_Commit() more than once");
    }

    // Maybe merge existing index data.
    if (num_seg_readers) {
        segs_merged = S_maybe_merge(self);
    }

    if (!segs_merged) {
        // Nothing merged.  Leave `needs_commit` false and bail out.
        ivars->prepared = true;
        return;
    }
    // Finish the segment and write a new snapshot file.
    else {
        Folder   *folder   = ivars->folder;
        Snapshot *snapshot = ivars->snapshot;

        // Write out new deletions.
        if (DelWriter_Updated(ivars->del_writer)) {
            // Only write out if they haven't all been applied.
            if (segs_merged != num_seg_readers) {
                DelWriter_Finish(ivars->del_writer);
            }
        }

        // Finish the segment.
        SegWriter_Finish(ivars->seg_writer);

        // Grab the write lock.
        S_obtain_write_lock(self);
        if (!ivars->write_lock) {
            RETHROW(INCREF(Err_get_error()));
        }

        // Write temporary snapshot file.
        DECREF(ivars->snapfile);
        String *snapfile = IxManager_Make_Snapshot_Filename(ivars->manager);
        ivars->snapfile = Str_Cat_Trusted_Utf8(snapfile, ".temp", 5);
        DECREF(snapfile);
        Folder_Delete(folder, ivars->snapfile);
        Snapshot_Write_File(snapshot, folder, ivars->snapfile);

        // Determine whether the index has been updated while this background
        // merge process was running.

        String *start_snapfile
            = Snapshot_Get_Path(PolyReader_Get_Snapshot(ivars->polyreader));
        Snapshot *latest_snapshot
            = Snapshot_Read_File(Snapshot_new(), ivars->folder, NULL);
        String *latest_snapfile = Snapshot_Get_Path(latest_snapshot);
        bool index_updated
            = !Str_Equals(start_snapfile, (Obj*)latest_snapfile);

        if (index_updated) {
            /* See if new deletions have been applied since this
             * background merge process started against any of the
             * segments we just merged away.  If that's true, we need to
             * write another segment which applies the deletions against
             * the new composite segment.
             */
            S_merge_updated_deletions(self);

            // Add the fresh content to our snapshot. (It's important to
            // run this AFTER S_merge_updated_deletions, because otherwise
            // we couldn't tell whether the deletion counts changed.)
            VArray *files = Snapshot_List(latest_snapshot);
            for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
                String *file = (String*)VA_Fetch(files, i);
                if (Str_Starts_With_Utf8(file, "seg_", 4)) {
                    int64_t gen = (int64_t)IxFileNames_extract_gen(file);
                    if (gen > ivars->cutoff) {
                        Snapshot_Add_Entry(ivars->snapshot, file);
                    }
                }
            }
            DECREF(files);

            // Since the snapshot content has changed, we need to rewrite it.
            Folder_Delete(folder, ivars->snapfile);
            Snapshot_Write_File(snapshot, folder, ivars->snapfile);
        }

        DECREF(latest_snapshot);

        ivars->needs_commit = true;
    }

    // Close reader, so that we can delete its files if appropriate.
    PolyReader_Close(ivars->polyreader);

    ivars->prepared = true;
}

void
BGMerger_Commit_IMP(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);

    // Safety check.
    if (!ivars->merge_lock) {
        THROW(ERR, "Can't call commit() more than once");
    }

    if (!ivars->prepared) {
        BGMerger_Prepare_Commit(self);
    }

    if (ivars->needs_commit) {
        bool success = false;
        String *temp_snapfile = ivars->snapfile;

        // Rename temp snapshot file.
        size_t ext_len      = sizeof(".temp") - 1;
        size_t snapfile_len = Str_Length(temp_snapfile);
        if (snapfile_len <= ext_len) {
            THROW(ERR, "Invalid snapfile name: %o", temp_snapfile);
        }
        ivars->snapfile = Str_SubString(temp_snapfile, 0,
                                       snapfile_len - ext_len);
        success = Folder_Hard_Link(ivars->folder, temp_snapfile,
                                   ivars->snapfile);
        Snapshot_Set_Path(ivars->snapshot, ivars->snapfile);
        if (!success) {
            String *mess = Str_newf("Can't create hard link from %o to %o",
                                    temp_snapfile, ivars->snapfile);
            DECREF(temp_snapfile);
            Err_throw_mess(ERR, mess);
        }
        if (!Folder_Delete(ivars->folder, temp_snapfile)) {
            String *mess = Str_newf("Can't delete %o", temp_snapfile);
            DECREF(temp_snapfile);
            Err_throw_mess(ERR, mess);
        }
        DECREF(temp_snapfile);
    }

    // Release the merge lock and remove the merge data file.
    S_release_merge_lock(self);
    IxManager_Remove_Merge_Data(ivars->manager);

    if (ivars->needs_commit) {
        // Purge obsolete files.
        FilePurger_Purge(ivars->file_purger);
    }

    // Release the write lock.
    S_release_write_lock(self);
}

static void
S_obtain_write_lock(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    Lock *write_lock = IxManager_Make_Write_Lock(ivars->manager);
    Lock_Clear_Stale(write_lock);
    if (Lock_Obtain(write_lock)) {
        // Only assign if successful, otherwise DESTROY unlocks -- bad!
        ivars->write_lock = write_lock;
    }
    else {
        DECREF(write_lock);
    }
}

static void
S_obtain_merge_lock(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    Lock *merge_lock = IxManager_Make_Merge_Lock(ivars->manager);
    Lock_Clear_Stale(merge_lock);
    if (Lock_Obtain(merge_lock)) {
        // Only assign if successful, same rationale as above.
        ivars->merge_lock = merge_lock;
    }
    else {
        // We can't get the merge lock, so it seems there must be another
        // BackgroundMerger running.
        DECREF(merge_lock);
    }
}

static void
S_release_write_lock(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    if (ivars->write_lock) {
        Lock_Release(ivars->write_lock);
        DECREF(ivars->write_lock);
        ivars->write_lock = NULL;
    }
}

static void
S_release_merge_lock(BackgroundMerger *self) {
    BackgroundMergerIVARS *const ivars = BGMerger_IVARS(self);
    if (ivars->merge_lock) {
        Lock_Release(ivars->merge_lock);
        DECREF(ivars->merge_lock);
        ivars->merge_lock = NULL;
    }
}


