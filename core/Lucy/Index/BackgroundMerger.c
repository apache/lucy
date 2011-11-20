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
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

// Verify a Folder or derive an FSFolder from a CharBuf path.
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
        = (BackgroundMerger*)VTable_Make_Obj(BACKGROUNDMERGER);
    return BGMerger_init(self, index, manager);
}

BackgroundMerger*
BGMerger_init(BackgroundMerger *self, Obj *index, IndexManager *manager) {
    Folder *folder = S_init_folder(index);

    // Init.
    self->optimize      = false;
    self->prepared      = false;
    self->needs_commit  = false;
    self->snapfile      = NULL;
    self->doc_maps      = Hash_new(0);

    // Assign.
    self->folder = folder;
    if (manager) {
        self->manager = (IndexManager*)INCREF(manager);
    }
    else {
        self->manager = IxManager_new(NULL, NULL);
        IxManager_Set_Write_Lock_Timeout(self->manager, 10000);
    }
    IxManager_Set_Folder(self->manager, folder);

    // Obtain write lock (which we'll only hold briefly), then merge lock.
    S_obtain_write_lock(self);
    if (!self->write_lock) {
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }
    S_obtain_merge_lock(self);
    if (!self->merge_lock) {
        DECREF(self);
        RETHROW(INCREF(Err_get_error()));
    }

    // Find the latest snapshot.  If there's no index content, bail early.
    self->snapshot = Snapshot_Read_File(Snapshot_new(), folder, NULL);
    if (!Snapshot_Get_Path(self->snapshot)) {
        S_release_write_lock(self);
        S_release_merge_lock(self);
        return self;
    }

    // Create FilePurger. Zap detritus from previous sessions.
    self->file_purger = FilePurger_new(folder, self->snapshot, self->manager);
    FilePurger_Purge(self->file_purger);

    // Open a PolyReader, passing in the IndexManager so we get a read lock on
    // the Snapshot's files -- so that Indexers don't zap our files while
    // we're operating in the background.
    self->polyreader = PolyReader_open((Obj*)folder, NULL, self->manager);

    // Clone the PolyReader's schema.
    Hash *dump = Schema_Dump(PolyReader_Get_Schema(self->polyreader));
    self->schema = (Schema*)CERTIFY(VTable_Load_Obj(SCHEMA, (Obj*)dump),
                                    SCHEMA);
    DECREF(dump);

    // Create new Segment.
    int64_t new_seg_num
        = IxManager_Highest_Seg_Num(self->manager, self->snapshot) + 1;
    VArray *fields = Schema_All_Fields(self->schema);
    self->segment = Seg_new(new_seg_num);
    for (uint32_t i = 0, max = VA_Get_Size(fields); i < max; i++) {
        Seg_Add_Field(self->segment, (CharBuf*)VA_Fetch(fields, i));
    }
    DECREF(fields);

    // Our "cutoff" is the segment this BackgroundMerger will write.  Now that
    // we've determined the cutoff, write the merge data file.
    self->cutoff = Seg_Get_Number(self->segment);
    IxManager_Write_Merge_Data(self->manager, self->cutoff);

    /* Create the SegWriter but hold off on preparing the new segment
     * directory -- because if we don't need to merge any segments we don't
     * need it.  (We've reserved the dir by plopping down the merge.json
     * file.) */
    self->seg_writer = SegWriter_new(self->schema, self->snapshot,
                                     self->segment, self->polyreader);

    // Grab a local ref to the DeletionsWriter.
    self->del_writer
        = (DeletionsWriter*)INCREF(SegWriter_Get_Del_Writer(self->seg_writer));

    // Release the write lock.  Now new Indexers can start while we work in
    // the background.
    S_release_write_lock(self);

    return self;
}

void
BGMerger_destroy(BackgroundMerger *self) {
    S_release_merge_lock(self);
    S_release_write_lock(self);
    DECREF(self->schema);
    DECREF(self->folder);
    DECREF(self->segment);
    DECREF(self->manager);
    DECREF(self->polyreader);
    DECREF(self->del_writer);
    DECREF(self->snapshot);
    DECREF(self->seg_writer);
    DECREF(self->file_purger);
    DECREF(self->write_lock);
    DECREF(self->snapfile);
    DECREF(self->doc_maps);
    SUPER_DESTROY(self, BACKGROUNDMERGER);
}

static Folder*
S_init_folder(Obj *index) {
    Folder *folder = NULL;

    // Validate or acquire a Folder.
    if (Obj_Is_A(index, FOLDER)) {
        folder = (Folder*)INCREF(index);
    }
    else if (Obj_Is_A(index, CHARBUF)) {
        folder = (Folder*)FSFolder_new((CharBuf*)index);
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
BGMerger_optimize(BackgroundMerger *self) {
    self->optimize = true;
}

static uint32_t
S_maybe_merge(BackgroundMerger *self) {
    VArray *to_merge = IxManager_Recycle(self->manager, self->polyreader,
                                         self->del_writer, 0, self->optimize);
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
    SegWriter_Prep_Seg_Dir(self->seg_writer);

    // Consolidate segments.
    for (uint32_t i = 0, max = num_to_merge; i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(to_merge, i);
        CharBuf   *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        int64_t    doc_count  = Seg_Get_Count(self->segment);
        Matcher *deletions
            = DelWriter_Seg_Deletions(self->del_writer, seg_reader);
        I32Array *doc_map = DelWriter_Generate_Doc_Map(
                                self->del_writer, deletions,
                                SegReader_Doc_Max(seg_reader),
                                (int32_t)doc_count);

        Hash_Store(self->doc_maps, (Obj*)seg_name, (Obj*)doc_map);
        SegWriter_Merge_Segment(self->seg_writer, seg_reader, doc_map);
        DECREF(deletions);
    }

    DECREF(to_merge);
    return num_to_merge;
}

static bool_t
S_merge_updated_deletions(BackgroundMerger *self) {
    Hash *updated_deletions = NULL;

    PolyReader *new_polyreader
        = PolyReader_open((Obj*)self->folder, NULL, NULL);
    VArray *new_seg_readers
        = PolyReader_Get_Seg_Readers(new_polyreader);
    VArray *old_seg_readers
        = PolyReader_Get_Seg_Readers(self->polyreader);
    Hash *new_segs = Hash_new(VA_Get_Size(new_seg_readers));

    for (uint32_t i = 0, max = VA_Get_Size(new_seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(new_seg_readers, i);
        CharBuf   *seg_name   = SegReader_Get_Seg_Name(seg_reader);
        Hash_Store(new_segs, (Obj*)seg_name, INCREF(seg_reader));
    }

    for (uint32_t i = 0, max = VA_Get_Size(old_seg_readers); i < max; i++) {
        SegReader *seg_reader = (SegReader*)VA_Fetch(old_seg_readers, i);
        CharBuf   *seg_name   = SegReader_Get_Seg_Name(seg_reader);

        // If this segment was merged away...
        if (Hash_Fetch(self->doc_maps, (Obj*)seg_name)) {
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
                          VTable_Get_Name(DELETIONSREADER));
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
            = PolyReader_open((Obj*)self->folder, self->snapshot, NULL);
        VArray *merge_seg_readers
            = PolyReader_Get_Seg_Readers(merge_polyreader);
        Snapshot *latest_snapshot
            = Snapshot_Read_File(Snapshot_new(), self->folder, NULL);
        int64_t new_seg_num
            = IxManager_Highest_Seg_Num(self->manager, latest_snapshot) + 1;
        Segment   *new_segment = Seg_new(new_seg_num);
        SegWriter *seg_writer  = SegWriter_new(self->schema, self->snapshot,
                                               new_segment, merge_polyreader);
        DeletionsWriter *del_writer = SegWriter_Get_Del_Writer(seg_writer);
        int64_t  merge_seg_num = Seg_Get_Number(self->segment);
        uint32_t seg_tick      = I32_MAX;
        int32_t  offset        = I32_MAX;
        CharBuf *seg_name      = NULL;
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
        if (offset == I32_MAX) { THROW(ERR, "Failed sanity check"); }

        Hash_Iterate(updated_deletions);
        while (Hash_Next(updated_deletions,
                         (Obj**)&seg_name, (Obj**)&deletions)
              ) {
            I32Array *doc_map
                = (I32Array*)CERTIFY(
                      Hash_Fetch(self->doc_maps, (Obj*)seg_name),
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
BGMerger_prepare_commit(BackgroundMerger *self) {
    VArray   *seg_readers     = PolyReader_Get_Seg_Readers(self->polyreader);
    uint32_t  num_seg_readers = VA_Get_Size(seg_readers);
    uint32_t  segs_merged     = 0;

    if (self->prepared) {
        THROW(ERR, "Can't call Prepare_Commit() more than once");
    }

    // Maybe merge existing index data.
    if (num_seg_readers) {
        segs_merged = S_maybe_merge(self);
    }

    if (!segs_merged) {
        // Nothing merged.  Leave self->needs_commit false and bail out.
        self->prepared = true;
        return;
    }
    // Finish the segment and write a new snapshot file.
    else {
        Folder   *folder   = self->folder;
        Snapshot *snapshot = self->snapshot;

        // Write out new deletions.
        if (DelWriter_Updated(self->del_writer)) {
            // Only write out if they haven't all been applied.
            if (segs_merged != num_seg_readers) {
                DelWriter_Finish(self->del_writer);
            }
        }

        // Finish the segment.
        SegWriter_Finish(self->seg_writer);

        // Grab the write lock.
        S_obtain_write_lock(self);
        if (!self->write_lock) {
            RETHROW(INCREF(Err_get_error()));
        }

        // Write temporary snapshot file.
        DECREF(self->snapfile);
        self->snapfile = IxManager_Make_Snapshot_Filename(self->manager);
        CB_Cat_Trusted_Str(self->snapfile, ".temp", 5);
        Folder_Delete(folder, self->snapfile);
        Snapshot_Write_File(snapshot, folder, self->snapfile);

        // Determine whether the index has been updated while this background
        // merge process was running.

        CharBuf *start_snapfile
            = Snapshot_Get_Path(PolyReader_Get_Snapshot(self->polyreader));
        Snapshot *latest_snapshot
            = Snapshot_Read_File(Snapshot_new(), self->folder, NULL);
        CharBuf *latest_snapfile = Snapshot_Get_Path(latest_snapshot);
        bool_t index_updated
            = !CB_Equals(start_snapfile, (Obj*)latest_snapfile);

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
                CharBuf *file = (CharBuf*)VA_Fetch(files, i);
                if (CB_Starts_With_Str(file, "seg_", 4)) {
                    int64_t gen = (int64_t)IxFileNames_extract_gen(file);
                    if (gen > self->cutoff) {
                        Snapshot_Add_Entry(self->snapshot, file);
                    }
                }
            }
            DECREF(files);

            // Since the snapshot content has changed, we need to rewrite it.
            Folder_Delete(folder, self->snapfile);
            Snapshot_Write_File(snapshot, folder, self->snapfile);
        }

        DECREF(latest_snapshot);

        self->needs_commit = true;
    }

    // Close reader, so that we can delete its files if appropriate.
    PolyReader_Close(self->polyreader);

    self->prepared = true;
}

void
BGMerger_commit(BackgroundMerger *self) {
    // Safety check.
    if (!self->merge_lock) {
        THROW(ERR, "Can't call commit() more than once");
    }

    if (!self->prepared) {
        BGMerger_Prepare_Commit(self);
    }

    if (self->needs_commit) {
        bool_t success = false;
        CharBuf *temp_snapfile = CB_Clone(self->snapfile);

        // Rename temp snapshot file.
        CB_Chop(self->snapfile, sizeof(".temp") - 1);
        success = Folder_Hard_Link(self->folder, temp_snapfile,
                                   self->snapfile);
        Snapshot_Set_Path(self->snapshot, self->snapfile);
        if (!success) {
            CharBuf *mess = CB_newf("Can't create hard link from %o to %o",
                                    temp_snapfile, self->snapfile);
            DECREF(temp_snapfile);
            Err_throw_mess(ERR, mess);
        }
        if (!Folder_Delete(self->folder, temp_snapfile)) {
            CharBuf *mess = CB_newf("Can't delete %o", temp_snapfile);
            DECREF(temp_snapfile);
            Err_throw_mess(ERR, mess);
        }
        DECREF(temp_snapfile);
    }

    // Release the merge lock and remove the merge data file.
    S_release_merge_lock(self);
    IxManager_Remove_Merge_Data(self->manager);

    if (self->needs_commit) {
        // Purge obsolete files.
        FilePurger_Purge(self->file_purger);
    }

    // Release the write lock.
    S_release_write_lock(self);
}

static void
S_obtain_write_lock(BackgroundMerger *self) {
    Lock *write_lock = IxManager_Make_Write_Lock(self->manager);
    Lock_Clear_Stale(write_lock);
    if (Lock_Obtain(write_lock)) {
        // Only assign if successful, otherwise DESTROY unlocks -- bad!
        self->write_lock = write_lock;
    }
    else {
        DECREF(write_lock);
    }
}

static void
S_obtain_merge_lock(BackgroundMerger *self) {
    Lock *merge_lock = IxManager_Make_Merge_Lock(self->manager);
    Lock_Clear_Stale(merge_lock);
    if (Lock_Obtain(merge_lock)) {
        // Only assign if successful, same rationale as above.
        self->merge_lock = merge_lock;
    }
    else {
        // We can't get the merge lock, so it seems there must be another
        // BackgroundMerger running.
        DECREF(merge_lock);
    }
}

static void
S_release_write_lock(BackgroundMerger *self) {
    if (self->write_lock) {
        Lock_Release(self->write_lock);
        DECREF(self->write_lock);
        self->write_lock = NULL;
    }
}

static void
S_release_merge_lock(BackgroundMerger *self) {
    if (self->merge_lock) {
        Lock_Release(self->merge_lock);
        DECREF(self->merge_lock);
        self->merge_lock = NULL;
    }
}


