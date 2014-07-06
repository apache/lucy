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
    FilePurger *self = (FilePurger*)Class_Make_Obj(FILEPURGER);
    return FilePurger_init(self, folder, snapshot, manager);
}

FilePurger*
FilePurger_init(FilePurger *self, Folder *folder, Snapshot *snapshot,
                IndexManager *manager) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    ivars->folder       = (Folder*)INCREF(folder);
    ivars->snapshot     = (Snapshot*)INCREF(snapshot);
    ivars->manager      = manager
                         ? (IndexManager*)INCREF(manager)
                         : IxManager_new(NULL, NULL);
    IxManager_Set_Folder(ivars->manager, folder);

    // Don't allow the locks directory to be zapped.
    ivars->disallowed = Hash_new(0);
    Hash_Store_Utf8(ivars->disallowed, "locks", 5, (Obj*)CFISH_TRUE);

    return self;
}

void
FilePurger_Destroy_IMP(FilePurger *self) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    DECREF(ivars->folder);
    DECREF(ivars->snapshot);
    DECREF(ivars->manager);
    DECREF(ivars->disallowed);
    SUPER_DESTROY(self, FILEPURGER);
}

void
FilePurger_Purge_IMP(FilePurger *self) {
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    Lock *deletion_lock = IxManager_Make_Deletion_Lock(ivars->manager);

    // Obtain deletion lock, purge files, release deletion lock.
    Lock_Clear_Stale(deletion_lock);
    if (Lock_Obtain(deletion_lock)) {
        Folder *folder   = ivars->folder;
        Hash   *failures = Hash_new(0);
        VArray *purgables;
        VArray *snapshots;

        S_discover_unused(self, &purgables, &snapshots);

        // Attempt to delete entries -- if failure, no big deal, just try
        // again later.  Proceed in reverse lexical order so that directories
        // get deleted after they've been emptied.
        VA_Sort(purgables, NULL, NULL);
        for (uint32_t i = VA_Get_Size(purgables); i--;) {
            String *entry = (String*)VA_Fetch(purgables, i);
            if (Hash_Fetch(ivars->disallowed, (Obj*)entry)) { continue; }
            if (!Folder_Delete(folder, entry)) {
                if (Folder_Exists(folder, entry)) {
                    Hash_Store(failures, (Obj*)entry, (Obj*)CFISH_TRUE);
                }
            }
        }

        for (uint32_t i = 0, max = VA_Get_Size(snapshots); i < max; i++) {
            Snapshot *snapshot = (Snapshot*)VA_Fetch(snapshots, i);
            bool snapshot_has_failures = false;
            if (Hash_Get_Size(failures)) {
                // Only delete snapshot files if all of their entries were
                // successfully deleted.
                VArray *entries = Snapshot_List(snapshot);
                for (uint32_t j = VA_Get_Size(entries); j--;) {
                    String *entry = (String*)VA_Fetch(entries, j);
                    if (Hash_Fetch(failures, (Obj*)entry)) {
                        snapshot_has_failures = true;
                        break;
                    }
                }
                DECREF(entries);
            }
            if (!snapshot_has_failures) {
                String *snapfile = Snapshot_Get_Path(snapshot);
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
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    IndexManager *manager    = ivars->manager;
    Lock         *merge_lock = IxManager_Make_Merge_Lock(manager);

    Lock_Clear_Stale(merge_lock);
    if (!Lock_Is_Locked(merge_lock)) {
        Hash *merge_data = IxManager_Read_Merge_Data(manager);
        Obj  *cutoff = merge_data
                       ? Hash_Fetch_Utf8(merge_data, "cutoff", 6)
                       : NULL;

        if (cutoff) {
            String *cutoff_seg = Seg_num_to_name(Obj_To_I64(cutoff));
            if (Folder_Exists(ivars->folder, cutoff_seg)) {
                StackString *merge_json = SSTR_WRAP_UTF8("merge.json", 10);
                DirHandle *dh = Folder_Open_Dir(ivars->folder, cutoff_seg);

                if (!dh) {
                    THROW(ERR, "Can't open segment dir '%o'", cutoff_seg);
                }

                Hash_Store(candidates, (Obj*)cutoff_seg, (Obj*)CFISH_TRUE);
                Hash_Store(candidates, (Obj*)merge_json, (Obj*)CFISH_TRUE);
                while (DH_Next(dh)) {
                    // TODO: recursively delete subdirs within seg dir.
                    String *entry = DH_Get_Entry(dh);
                    String *filepath = Str_newf("%o/%o", cutoff_seg, entry);
                    Hash_Store(candidates, (Obj*)filepath, (Obj*)CFISH_TRUE);
                    DECREF(filepath);
                    DECREF(entry);
                }
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
    FilePurgerIVARS *const ivars = FilePurger_IVARS(self);
    Folder      *folder       = ivars->folder;
    DirHandle   *dh           = Folder_Open_Dir(folder, NULL);
    if (!dh) { RETHROW(INCREF(Err_get_error())); }
    VArray      *spared       = VA_new(1);
    VArray      *snapshots    = VA_new(1);
    String      *snapfile     = NULL;

    // Start off with the list of files in the current snapshot.
    if (ivars->snapshot) {
        VArray *entries    = Snapshot_List(ivars->snapshot);
        VArray *referenced = S_find_all_referenced(folder, entries);
        VA_Push_VArray(spared, referenced);
        DECREF(entries);
        DECREF(referenced);
        snapfile = Snapshot_Get_Path(ivars->snapshot);
        if (snapfile) { VA_Push(spared, INCREF(snapfile)); }
    }

    Hash *candidates = Hash_new(64);
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
                VA_Push(spared, (Obj*)Str_Clone(entry));
                VA_Push_VArray(spared, referenced);
            }
            else {
                // No one's using this snapshot, so all of its entries are
                // candidates for deletion.
                for (uint32_t i = 0, max = VA_Get_Size(referenced); i < max; i++) {
                    String *file = (String*)VA_Fetch(referenced, i);
                    Hash_Store(candidates, (Obj*)file, (Obj*)CFISH_TRUE);
                }
                VA_Push(snapshots, INCREF(snapshot));
            }

            DECREF(referenced);
            DECREF(snap_list);
            DECREF(snapshot);
            DECREF(lock);
        }
        DECREF(entry);
    }
    DECREF(dh);

    // Clean up after a dead segment consolidation.
    S_zap_dead_merge(self, candidates);

    // Eliminate any current files from the list of files to be purged.
    for (uint32_t i = 0, max = VA_Get_Size(spared); i < max; i++) {
        String *filename = (String*)VA_Fetch(spared, i);
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
        String *entry = (String*)VA_Fetch(entries, i);
        Hash_Store(uniqued, (Obj*)entry, (Obj*)CFISH_TRUE);
        if (Folder_Is_Directory(folder, entry)) {
            VArray *contents = Folder_List_R(folder, entry);
            for (uint32_t j = VA_Get_Size(contents); j--;) {
                String *sub_entry = (String*)VA_Fetch(contents, j);
                Hash_Store(uniqued, (Obj*)sub_entry, (Obj*)CFISH_TRUE);
            }
            DECREF(contents);
        }
    }
    VArray *referenced = Hash_Keys(uniqued);
    DECREF(uniqued);
    return referenced;
}


