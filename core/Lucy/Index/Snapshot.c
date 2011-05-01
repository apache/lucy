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
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/StringHelper.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

static VArray*
S_clean_segment_contents(VArray *orig);

int32_t Snapshot_current_file_format = 2;
static int32_t Snapshot_current_file_subformat = 1;

Snapshot*
Snapshot_new() {
    Snapshot *self = (Snapshot*)VTable_Make_Obj(SNAPSHOT);
    return Snapshot_init(self);
}

static void
S_zero_out(Snapshot *self) {
    DECREF(self->entries);
    DECREF(self->path);
    self->entries  = Hash_new(0);
    self->path = NULL;
}

Snapshot*
Snapshot_init(Snapshot *self) {
    S_zero_out(self);
    return self;
}

void
Snapshot_destroy(Snapshot *self) {
    DECREF(self->entries);
    DECREF(self->path);
    SUPER_DESTROY(self, SNAPSHOT);
}

void
Snapshot_add_entry(Snapshot *self, const CharBuf *entry) {
    Hash_Store(self->entries, (Obj*)entry, INCREF(&EMPTY));
}

bool_t
Snapshot_delete_entry(Snapshot *self, const CharBuf *entry) {
    Obj *val = Hash_Delete(self->entries, (Obj*)entry);
    if (val) {
        Obj_Dec_RefCount(val);
        return true;
    }
    else {
        return false;
    }
}

VArray*
Snapshot_list(Snapshot *self) {
    return Hash_Keys(self->entries);
}

uint32_t
Snapshot_num_entries(Snapshot *self) {
    return Hash_Get_Size(self->entries);
}

void
Snapshot_set_path(Snapshot *self, const CharBuf *path) {
    DECREF(self->path);
    self->path = path ? CB_Clone(path) : NULL;
}

CharBuf*
Snapshot_get_path(Snapshot *self) {
    return self->path;
}

Snapshot*
Snapshot_read_file(Snapshot *self, Folder *folder, const CharBuf *path) {
    // Eliminate all prior data. Pick a snapshot file.
    S_zero_out(self);
    self->path = path ? CB_Clone(path) : IxFileNames_latest_snapshot(folder);

    if (self->path) {
        Hash *snap_data
            = (Hash*)CERTIFY(Json_slurp_json(folder, self->path), HASH);
        Obj *format_obj
            = CERTIFY(Hash_Fetch_Str(snap_data, "format", 6), OBJ);
        int32_t format = (int32_t)Obj_To_I64(format_obj);
        Obj *subformat_obj = Hash_Fetch_Str(snap_data, "subformat", 9);
        int32_t subformat = subformat_obj
                            ? (int32_t)Obj_To_I64(subformat_obj)
                            : 0;

        // Verify that we can read the index properly.
        if (format > Snapshot_current_file_format) {
            THROW(ERR, "Snapshot format too recent: %i32, %i32",
                  format, Snapshot_current_file_format);
        }

        // Build up list of entries.
        VArray *list = (VArray*)CERTIFY(
                           Hash_Fetch_Str(snap_data, "entries", 7),
                           VARRAY);
        INCREF(list);
        if (format == 1 || (format == 2 && subformat < 1)) {
            VArray *cleaned = S_clean_segment_contents(list);
            DECREF(list);
            list = cleaned;
        }
        Hash_Clear(self->entries);
        for (uint32_t i = 0, max = VA_Get_Size(list); i < max; i++) {
            CharBuf *entry
                = (CharBuf*)CERTIFY(VA_Fetch(list, i), CHARBUF);
            Hash_Store(self->entries, (Obj*)entry, INCREF(&EMPTY));
        }

        DECREF(list);
        DECREF(snap_data);
    }

    return self;
}

static VArray*
S_clean_segment_contents(VArray *orig) {
    // Since Snapshot format 2, no DataReader has depended on individual files
    // within segment directories being listed.  Filter these files because
    // they cause a problem with FilePurger.
    VArray *cleaned = VA_new(VA_Get_Size(orig));
    for (uint32_t i = 0, max = VA_Get_Size(orig); i < max; i++) {
        CharBuf *name = (CharBuf*)VA_Fetch(orig, i);
        if (!Seg_valid_seg_name(name)) {
            if (CB_Starts_With_Str(name, "seg_", 4)) {
                continue;  // Skip this file.
            }
        }
        VA_Push(cleaned, INCREF(name));
    }
    return cleaned;
}


void
Snapshot_write_file(Snapshot *self, Folder *folder, const CharBuf *path) {
    Hash   *all_data = Hash_new(0);
    VArray *list     = Snapshot_List(self);

    // Update path.
    DECREF(self->path);
    if (path) {
        self->path = CB_Clone(path);
    }
    else {
        CharBuf *latest = IxFileNames_latest_snapshot(folder);
        uint64_t gen = latest ? IxFileNames_extract_gen(latest) + 1 : 1;
        char base36[StrHelp_MAX_BASE36_BYTES];
        StrHelp_to_base36(gen, &base36);
        self->path = CB_newf("snapshot_%s.json", &base36);
        DECREF(latest);
    }

    // Don't overwrite.
    if (Folder_Exists(folder, self->path)) {
        THROW(ERR, "Snapshot file '%o' already exists", self->path);
    }

    // Sort, then store file names.
    VA_Sort(list, NULL, NULL);
    Hash_Store_Str(all_data, "entries", 7, (Obj*)list);

    // Create a JSON-izable data structure.
    Hash_Store_Str(all_data, "format", 6,
                   (Obj*)CB_newf("%i32", (int32_t)Snapshot_current_file_format));
    Hash_Store_Str(all_data, "subformat", 9,
                   (Obj*)CB_newf("%i32", (int32_t)Snapshot_current_file_subformat));

    // Write out JSON-ized data to the new file.
    Json_spew_json((Obj*)all_data, folder, self->path);

    DECREF(all_data);
}


