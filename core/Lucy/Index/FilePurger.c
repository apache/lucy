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
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/FilePurger.h"
#include "Clownfish/Boolean.h"
#include "Clownfish/HashIterator.h"
#include "Lucy/Index/IndexManager.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/Lock.h"
#include "Lucy/Util/Json.h"

// Add unused files to purged hash, used files to spared hash, and
// obsolete snapshots to snapshots array.
static void
S_discover_unused(FilePurger *self, Snapshot *current, Hash *spared,
                  Hash *purged, Vector *snapshots);

// Add filepath entries referenced by a snapshot to a Hash. Note that
// it's assumed that snapshots only list entries local to the index
// folder.
static void
S_find_all_referenced(Snapshot *snapshot, Hash *set);

// Delete local entry from a folder. Handles CompoundFileReaders efficiently
// but doesn't support subdirectories in 'entry'.
static bool
S_delete_entry(Folder *folder, String *entry);

FilePurger*
FilePurger_new(Folder *folder, IndexManager *manager) {
    FilePurger *self = (FilePurger*)Class_Make_Obj(FILEPURGER);
    return FilePurger_init(self, folder, manager);
}

FilePurger*
FilePurger_init(FilePurger *self, Folder *folder, IndexManager *manager) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    ivars->folder       = (Folder*)INCREF(folder);
    ivars->manager      = manager
                         ? (IndexManager*)INCREF(manager)
                         : IxManager_new(NULL, NULL);
    IxManager_Set_Folder(ivars->manager, folder);

    return self;
}

void
FilePurger_Destroy_IMP(FilePurger *self) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    DECREF(ivars->folder);
    DECREF(ivars->manager);
    SUPER_DESTROY(self, FILEPURGER);
}

void
FilePurger_Purge_Snapshots_IMP(FilePurger *self, Snapshot *current) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    Lock *deletion_lock = IxManager_Make_Deletion_Lock(ivars->manager);

    // Obtain deletion lock, purge files, release deletion lock.
    if (Lock_Obtain_Exclusive(deletion_lock)) {
        Folder *folder    = ivars->folder;
        Hash   *failures  = Hash_new(16);
        Hash   *spared    = Hash_new(32);
        Hash   *purged    = Hash_new(32);
        Vector *snapshots = Vec_new(16);

        // Don't allow the locks directory to be zapped.
        Hash_Store_Utf8(spared, "locks", 5, (Obj*)CFISH_TRUE);

        S_discover_unused(self, current, spared, purged, snapshots);

        // Attempt to delete entries -- if failure, no big deal, just try
        // again later.
        HashIterator *iter = HashIter_new(purged);
        while (HashIter_Next(iter)) {
            String *entry = HashIter_Get_Key(iter);
            if (Hash_Fetch(spared, entry)) { continue; }
            if (!S_delete_entry(folder, entry)) {
                if (Folder_Exists(folder, entry)) {
                    Hash_Store(failures, entry, (Obj*)CFISH_TRUE);
                }
            }
        }

        for (size_t i = 0, max = Vec_Get_Size(snapshots); i < max; i++) {
            Snapshot *snapshot = (Snapshot*)Vec_Fetch(snapshots, i);
            bool snapshot_has_failures = false;
            if (Hash_Get_Size(failures)) {
                // Only delete snapshot files if all of their entries were
                // successfully deleted.
                Vector *entries = Snapshot_List(snapshot);
                for (size_t j = Vec_Get_Size(entries); j--;) {
                    String *entry = (String*)Vec_Fetch(entries, j);
                    if (Hash_Fetch(failures, entry)) {
                        snapshot_has_failures = true;
                        break;
                    }
                }
                DECREF(entries);
            }
            if (!snapshot_has_failures) {
                String *snapfile = Snapshot_Get_Path(snapshot);
                Folder_Local_Delete(folder, snapfile);
            }
        }

        DECREF(iter);
        DECREF(failures);
        DECREF(purged);
        DECREF(spared);
        DECREF(snapshots);
        Lock_Release(deletion_lock);
    }
    else {
        WARN("Can't obtain deletion lock, skipping deletion of "
             "obsolete files");
    }

    DECREF(deletion_lock);
}

void
FilePurger_Purge_Aborted_Merge_IMP(FilePurger *self) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    IndexManager *manager    = ivars->manager;
    Lock         *merge_lock = IxManager_Make_Merge_Lock(manager);

    if (!Lock_Is_Locked_Exclusive(merge_lock)) {
        Hash *merge_data = IxManager_Read_Merge_Data(manager);
        Obj  *cutoff = merge_data
                       ? Hash_Fetch_Utf8(merge_data, "cutoff", 6)
                       : NULL;

        if (cutoff) {
            Folder *folder = ivars->folder;

            String *cutoff_seg = Seg_num_to_name(Json_obj_to_i64(cutoff));
            if (Folder_Local_Exists(folder, cutoff_seg)) {
                if (!S_delete_entry(folder, cutoff_seg)) {
                    if (Folder_Local_Exists(folder, cutoff_seg)) {
                        WARN("Couldn't delete '%o' from aborted merge",
                             cutoff_seg);
                    }
                }
            }

            String *merge_json = SSTR_WRAP_C("merge.json");
            if (!Folder_Local_Delete(folder, merge_json)) {
                if (Folder_Local_Exists(folder, merge_json)) {
                    WARN("Couldn't delete '%o' from aborted merge",
                         merge_json);
                }
            }

            DECREF(cutoff_seg);
        }

        DECREF(merge_data);
    }

    DECREF(merge_lock);
    return;
}

static void
S_discover_unused(FilePurger *self, Snapshot *current, Hash *spared,
                  Hash *purged, Vector *snapshots) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    Folder      *folder       = ivars->folder;
    DirHandle   *dh           = Folder_Open_Dir(folder, NULL);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }
    String      *snapfile     = Snapshot_Get_Path(current);

    snapfile = Snapshot_Get_Path(current);
    if (snapfile) {
        Hash_Store(spared, snapfile, (Obj*)CFISH_TRUE);
    }
    S_find_all_referenced(current, spared);

    while (DH_Next(dh)) {
        String *entry = DH_Get_Entry(dh);
        if (Str_Starts_With_Utf8(entry, "snapshot_", 9)
            && Str_Ends_With_Utf8(entry, ".json", 5)
            && (!snapfile || !Str_Equals(entry, (Obj*)snapfile))
        ) {
            Snapshot *snapshot
                = Snapshot_Read_File(Snapshot_new(), folder, entry);
            Lock *lock
                = IxManager_Make_Snapshot_Read_Lock(ivars->manager, entry);

            // DON'T obtain the lock -- only see whether another
            // entity holds a lock on the snapshot file.
            if (lock && Lock_Is_Locked(lock)) {
                // The snapshot file is locked, which means someone's using
                // that version of the index -- protect all of its entries.
                Hash_Store(spared, entry, (Obj*)CFISH_TRUE);
                S_find_all_referenced(snapshot, spared);
            }
            else {
                // No one's using this snapshot, so all of its entries are
                // candidates for deletion.
                Vec_Push(snapshots, INCREF(snapshot));
                S_find_all_referenced(snapshot, purged);
            }

            DECREF(snapshot);
            DECREF(lock);
        }
        DECREF(entry);
    }

    DECREF(dh);
}

static void
S_find_all_referenced(Snapshot *snapshot, Hash *set) {
    Vector *snap_list = Snapshot_List(snapshot);

    for (size_t i = 0, max = Vec_Get_Size(snap_list); i < max; i++) {
        String *entry = (String*)Vec_Fetch(snap_list, i);
        Hash_Store(set, entry, (Obj*)CFISH_TRUE);
    }

    DECREF(snap_list);
}

static bool
S_delete_entry(Folder *folder, String *folder_entry) {
    if (Folder_Local_Is_Directory(folder, folder_entry)) {
        // CFReader has the nasty habit of listing both real and virtual
        // files. Get the real folder.
        Folder *inner = Folder_Local_Find_Folder(folder, folder_entry);
        if (inner == NULL) { return false; }
        if (Folder_is_a(inner, COMPOUNDFILEREADER)) {
            inner = CFReader_Get_Real_Folder((CompoundFileReader*)inner);
        }

        Vector *entries = Folder_List(inner, NULL);
        if (entries == NULL) { return false; }

        for (size_t i = 0, max = Vec_Get_Size(entries); i < max; i++) {
            // TODO: recursively delete subdirs within seg dir.
            String *entry = (String*)Vec_Fetch(entries, i);
            Folder_Local_Delete(inner, entry);
        }

        DECREF(entries);
    }

    return Folder_Local_Delete(folder, folder_entry);
}

