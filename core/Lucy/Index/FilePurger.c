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

#define C_LUCY_FILEPURGER
#include <ctype.h>
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/FilePurger.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Plan/Schema.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/Lock.h"

// Place unused files into purgables array and obsolete Snapshots into
// snapshots array.
static void
S_discover_unused(FilePurger *self, VArray **purgables, VArray **snapshots);

// Clean up after a failed background merge session, adding all dead files to
// the list of candidates to be zapped.
static void
S_zap_dead_merge(FilePurger *self, Hash *candidates);

// Return an array of recursively expanded filepath entries.
static VArray*
S_find_all_referenced(Folder *folder, VArray *entries);

FilePurger*
FilePurger_new(Folder *folder, Snapshot *snapshot, IndexManager *manager) {
    FilePurger *self = (FilePurger*)VTable_Make_Obj(FILEPURGER);
    return FilePurger_init(self, folder, snapshot, manager);
}

FilePurger*
FilePurger_init(FilePurger *self, Folder *folder, Snapshot *snapshot,
                IndexManager *manager) {
    self->folder       = (Folder*)INCREF(folder);
    self->snapshot     = (Snapshot*)INCREF(snapshot);
    self->manager      = manager
                         ? (IndexManager*)INCREF(manager)
                         : IxManager_new(NULL, NULL);
    IxManager_Set_Folder(self->manager, folder);

    // Don't allow the locks directory to be zapped.
    self->disallowed = Hash_new(0);
    Hash_Store_Str(self->disallowed, "locks", 5, INCREF(&EMPTY));

    return self;
}

void
FilePurger_destroy(FilePurger *self) {
    DECREF(self->folder);
    DECREF(self->snapshot);
    DECREF(self->manager);
    DECREF(self->disallowed);
    SUPER_DESTROY(self, FILEPURGER);
}

void
FilePurger_purge(FilePurger *self) {
    Lock *deletion_lock = IxManager_Make_Deletion_Lock(self->manager);

    // Obtain deletion lock, purge files, release deletion lock.
    Lock_Clear_Stale(deletion_lock);
    if (Lock_Obtain(deletion_lock)) {
        Folder *folder   = self->folder;
        Hash   *failures = Hash_new(0);
        VArray *purgables;
        VArray *snapshots;

        S_discover_unused(self, &purgables, &snapshots);

        // Attempt to delete entries -- if failure, no big deal, just try
        // again later.  Proceed in reverse lexical order so that directories
        // get deleted after they've been emptied.
        VA_Sort(purgables, NULL, NULL);
        for (uint32_t i = VA_Get_Size(purgables); i--;) {
            CharBuf *entry = (CharBuf*)VA_fetch(purgables, i);
            if (Hash_Fetch(self->disallowed, (Obj*)entry)) { continue; }
            if (!Folder_Delete(folder, entry)) {
                if (Folder_Exists(folder, entry)) {
                    Hash_Store(failures, (Obj*)entry, INCREF(&EMPTY));
                }
            }
        }

        for (uint32_t i = 0, max = VA_Get_Size(snapshots); i < max; i++) {
            Snapshot *snapshot = (Snapshot*)VA_Fetch(snapshots, i);
            bool_t snapshot_has_failures = false;
            if (Hash_Get_Size(failures)) {
                // Only delete snapshot files if all of their entries were
                // successfully deleted.
                VArray *entries = Snapshot_List(snapshot);
                for (uint32_t j = VA_Get_Size(entries); j--;) {
                    CharBuf *entry = (CharBuf*)VA_Fetch(entries, j);
                    if (Hash_Fetch(failures, (Obj*)entry)) {
                        snapshot_has_failures = true;
                        break;
                    }
                }
                DECREF(entries);
            }
            if (!snapshot_has_failures) {
                CharBuf *snapfile = Snapshot_Get_Path(snapshot);
                Folder_Delete(folder, snapfile);
            }
        }

        DECREF(failures);
        DECREF(purgables);
        DECREF(snapshots);
        Lock_Release(deletion_lock);
    }
    else {
        WARN("Can't obtain deletion lock, skipping deletion of "
             "obsolete files");
    }

    DECREF(deletion_lock);
}

static void
S_zap_dead_merge(FilePurger *self, Hash *candidates) {
    IndexManager *manager    = self->manager;
    Lock         *merge_lock = IxManager_Make_Merge_Lock(manager);

    Lock_Clear_Stale(merge_lock);
    if (!Lock_Is_Locked(merge_lock)) {
        Hash *merge_data = IxManager_Read_Merge_Data(manager);
        Obj  *cutoff = merge_data
                       ? Hash_Fetch_Str(merge_data, "cutoff", 6)
                       : NULL;

        if (cutoff) {
            CharBuf *cutoff_seg = Seg_num_to_name(Obj_To_I64(cutoff));
            if (Folder_Exists(self->folder, cutoff_seg)) {
                ZombieCharBuf *merge_json = ZCB_WRAP_STR("merge.json", 10);
                DirHandle *dh = Folder_Open_Dir(self->folder, cutoff_seg);
                CharBuf *entry = dh ? DH_Get_Entry(dh) : NULL;
                CharBuf *filepath = CB_new(32);

                if (!dh) {
                    THROW(ERR, "Can't open segment dir '%o'", filepath);
                }

                Hash_Store(candidates, (Obj*)cutoff_seg, INCREF(&EMPTY));
                Hash_Store(candidates, (Obj*)merge_json, INCREF(&EMPTY));
                while (DH_Next(dh)) {
                    // TODO: recursively delete subdirs within seg dir.
                    CB_setf(filepath, "%o/%o", cutoff_seg, entry);
                    Hash_Store(candidates, (Obj*)filepath, INCREF(&EMPTY));
                }
                DECREF(filepath);
                DECREF(dh);
            }
            DECREF(cutoff_seg);
        }

        DECREF(merge_data);
    }

    DECREF(merge_lock);
    return;
}

static void
S_discover_unused(FilePurger *self, VArray **purgables_ptr,
                  VArray **snapshots_ptr) {
    Folder      *folder       = self->folder;
    DirHandle   *dh           = Folder_Open_Dir(folder, NULL);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }
    VArray      *spared       = VA_new(1);
    VArray      *snapshots    = VA_new(1);
    CharBuf     *snapfile     = NULL;

    // Start off with the list of files in the current snapshot.
    if (self->snapshot) {
        VArray *entries    = Snapshot_List(self->snapshot);
        VArray *referenced = S_find_all_referenced(folder, entries);
        VA_Push_VArray(spared, referenced);
        DECREF(entries);
        DECREF(referenced);
        snapfile = Snapshot_Get_Path(self->snapshot);
        if (snapfile) { VA_Push(spared, INCREF(snapfile)); }
    }

    CharBuf *entry      = DH_Get_Entry(dh);
    Hash    *candidates = Hash_new(64);
    while (DH_Next(dh)) {
        if (!CB_Starts_With_Str(entry, "snapshot_", 9))        { continue; }
        else if (!CB_Ends_With_Str(entry, ".json", 5))         { continue; }
        else if (snapfile && CB_Equals(entry, (Obj*)snapfile)) { continue; }
        else {
            Snapshot *snapshot
                = Snapshot_Read_File(Snapshot_new(), folder, entry);
            Lock *lock
                = IxManager_Make_Snapshot_Read_Lock(self->manager, entry);
            VArray *snap_list  = Snapshot_List(snapshot);
            VArray *referenced = S_find_all_referenced(folder, snap_list);

            // DON'T obtain the lock -- only see whether another
            // entity holds a lock on the snapshot file.
            if (lock) {
                Lock_Clear_Stale(lock);
            }
            if (lock && Lock_Is_Locked(lock)) {
                // The snapshot file is locked, which means someone's using
                // that version of the index -- protect all of its entries.
                uint32_t new_size = VA_Get_Size(spared)
                                    + VA_Get_Size(referenced)
                                    + 1;
                VA_Grow(spared, new_size);
                VA_Push(spared, (Obj*)CB_Clone(entry));
                VA_Push_VArray(spared, referenced);
            }
            else {
                // No one's using this snapshot, so all of its entries are
                // candidates for deletion.
                for (uint32_t i = 0, max = VA_Get_Size(referenced); i < max; i++) {
                    CharBuf *file = (CharBuf*)VA_Fetch(referenced, i);
                    Hash_Store(candidates, (Obj*)file, INCREF(&EMPTY));
                }
                VA_Push(snapshots, INCREF(snapshot));
            }

            DECREF(referenced);
            DECREF(snap_list);
            DECREF(snapshot);
            DECREF(lock);
        }
    }
    DECREF(dh);

    // Clean up after a dead segment consolidation.
    S_zap_dead_merge(self, candidates);

    // Eliminate any current files from the list of files to be purged.
    for (uint32_t i = 0, max = VA_Get_Size(spared); i < max; i++) {
        CharBuf *filename = (CharBuf*)VA_Fetch(spared, i);
        DECREF(Hash_Delete(candidates, (Obj*)filename));
    }

    // Pass back purgables and Snapshots.
    *purgables_ptr = Hash_Keys(candidates);
    *snapshots_ptr = snapshots;

    DECREF(candidates);
    DECREF(spared);
}

static VArray*
S_find_all_referenced(Folder *folder, VArray *entries) {
    Hash *uniqued = Hash_new(VA_Get_Size(entries));
    for (uint32_t i = 0, max = VA_Get_Size(entries); i < max; i++) {
        CharBuf *entry = (CharBuf*)VA_Fetch(entries, i);
        Hash_Store(uniqued, (Obj*)entry, INCREF(&EMPTY));
        if (Folder_Is_Directory(folder, entry)) {
            VArray *contents = Folder_List_R(folder, entry);
            for (uint32_t j = VA_Get_Size(contents); j--;) {
                CharBuf *sub_entry = (CharBuf*)VA_Fetch(contents, j);
                Hash_Store(uniqued, (Obj*)sub_entry, INCREF(&EMPTY));
            }
            DECREF(contents);
        }
    }
    VArray *referenced = Hash_Keys(uniqued);
    DECREF(uniqued);
    return referenced;
}


