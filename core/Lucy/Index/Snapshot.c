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

#define C_LUCY_SNAPSHOT
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Snapshot.h"
#include "Clownfish/Boolean.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/Folder.h"
#include "Clownfish/Util/StringHelper.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

static Vector*
S_clean_segment_contents(Vector *orig);

int32_t Snapshot_current_file_format = 2;
static int32_t Snapshot_current_file_subformat = 1;

Snapshot*
Snapshot_new() {
    Snapshot *self = (Snapshot*)Class_Make_Obj(SNAPSHOT);
    return Snapshot_init(self);
}

static void
S_zero_out(Snapshot *self) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    DECREF(ivars->entries);
    DECREF(ivars->path);
    ivars->entries  = Hash_new(0);
    ivars->path = NULL;
}

Snapshot*
Snapshot_init(Snapshot *self) {
    S_zero_out(self);
    return self;
}

void
Snapshot_Destroy_IMP(Snapshot *self) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    DECREF(ivars->entries);
    DECREF(ivars->path);
    SUPER_DESTROY(self, SNAPSHOT);
}

void
Snapshot_Add_Entry_IMP(Snapshot *self, String *entry) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    Hash_Store(ivars->entries, entry, (Obj*)CFISH_TRUE);
}

bool
Snapshot_Delete_Entry_IMP(Snapshot *self, String *entry) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    Obj *val = Hash_Delete(ivars->entries, entry);
    if (val) {
        DECREF(val);
        return true;
    }
    else {
        return false;
    }
}

Vector*
Snapshot_List_IMP(Snapshot *self) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    return Hash_Keys(ivars->entries);
}

uint32_t
Snapshot_Num_Entries_IMP(Snapshot *self) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    return Hash_Get_Size(ivars->entries);
}

void
Snapshot_Set_Path_IMP(Snapshot *self, String *path) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    String *temp = ivars->path;
    ivars->path = path ? Str_Clone(path) : NULL;
    DECREF(temp);
}

String*
Snapshot_Get_Path_IMP(Snapshot *self) {
    return Snapshot_IVARS(self)->path;
}

Snapshot*
Snapshot_Read_File_IMP(Snapshot *self, Folder *folder, String *path) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);

    // Eliminate all prior data. Pick a snapshot file.
    S_zero_out(self);
    ivars->path = (path != NULL && Str_Get_Size(path) > 0)
                  ? Str_Clone(path)
                  : IxFileNames_latest_snapshot(folder);

    if (ivars->path) {
        Hash *snap_data
            = (Hash*)CERTIFY(Json_slurp_json(folder, ivars->path), HASH);
        Obj *format_obj
            = CERTIFY(Hash_Fetch_Utf8(snap_data, "format", 6), OBJ);
        int32_t format = (int32_t)Json_obj_to_i64(format_obj);
        Obj *subformat_obj = Hash_Fetch_Utf8(snap_data, "subformat", 9);
        int32_t subformat = subformat_obj
                            ? (int32_t)Json_obj_to_i64(subformat_obj)
                            : 0;

        // Verify that we can read the index properly.
        if (format > Snapshot_current_file_format) {
            THROW(ERR, "Snapshot format too recent: %i32, %i32",
                  format, Snapshot_current_file_format);
        }

        // Build up list of entries.
        Vector *list = (Vector*)INCREF(CERTIFY(
                           Hash_Fetch_Utf8(snap_data, "entries", 7),
                           VECTOR));
        if (format == 1 || (format == 2 && subformat < 1)) {
            Vector *cleaned = S_clean_segment_contents(list);
            DECREF(list);
            list = cleaned;
        }
        Hash_Clear(ivars->entries);
        for (size_t i = 0, max = Vec_Get_Size(list); i < max; i++) {
            String *entry
                = (String*)CERTIFY(Vec_Fetch(list, i), STRING);
            Hash_Store(ivars->entries, entry, (Obj*)CFISH_TRUE);
        }

        DECREF(list);
        DECREF(snap_data);
    }

    return self;
}

static Vector*
S_clean_segment_contents(Vector *orig) {
    // Since Snapshot format 2, no DataReader has depended on individual files
    // within segment directories being listed.  Filter these files because
    // they cause a problem with FilePurger.
    Vector *cleaned = Vec_new(Vec_Get_Size(orig));
    for (size_t i = 0, max = Vec_Get_Size(orig); i < max; i++) {
        String *name = (String*)Vec_Fetch(orig, i);
        if (!Seg_valid_seg_name(name)) {
            if (Str_Starts_With_Utf8(name, "seg_", 4)) {
                continue;  // Skip this file.
            }
        }
        Vec_Push(cleaned, INCREF(name));
    }
    return cleaned;
}


void
Snapshot_Write_File_IMP(Snapshot *self, Folder *folder, String *path) {
    SnapshotIVARS *const ivars = Snapshot_IVARS(self);
    Hash   *all_data = Hash_new(0);
    Vector *list     = Snapshot_List(self);

    // Update path.
    DECREF(ivars->path);
    if (path != NULL && Str_Get_Size(path) != 0) {
        ivars->path = Str_Clone(path);
    }
    else {
        String *latest = IxFileNames_latest_snapshot(folder);
        uint64_t gen = latest ? IxFileNames_extract_gen(latest) + 1 : 1;
        char base36[StrHelp_MAX_BASE36_BYTES];
        StrHelp_to_base36(gen, &base36);
        ivars->path = Str_newf("snapshot_%s.json", &base36);
        DECREF(latest);
    }

    // Don't overwrite.
    if (Folder_Exists(folder, ivars->path)) {
        THROW(ERR, "Snapshot file '%o' already exists", ivars->path);
    }

    // Sort, then store file names.
    Vec_Sort(list);
    Hash_Store_Utf8(all_data, "entries", 7, (Obj*)list);

    // Create a JSON-izable data structure.
    Hash_Store_Utf8(all_data, "format", 6,
                    (Obj*)Str_newf("%i32", (int32_t)Snapshot_current_file_format));
    Hash_Store_Utf8(all_data, "subformat", 9,
                    (Obj*)Str_newf("%i32", (int32_t)Snapshot_current_file_subformat));

    // Write out JSON-ized data to the new file.
    Json_spew_json((Obj*)all_data, folder, ivars->path);

    DECREF(all_data);
}


