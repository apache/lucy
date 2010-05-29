#define C_LUCY_SNAPSHOT
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Snapshot.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Util/StringHelper.h"
#include "Lucy/Util/IndexFileNames.h"
#include "Lucy/Util/Json.h"

int32_t Snapshot_current_file_format = 2;

Snapshot*
Snapshot_new()
{
    Snapshot *self = (Snapshot*)VTable_Make_Obj(SNAPSHOT);
    return Snapshot_init(self);
}

static void
S_zero_out(Snapshot *self)
{
    DECREF(self->entries);
    DECREF(self->path);
    self->entries  = Hash_new(0);
    self->path = NULL;
}

Snapshot*
Snapshot_init(Snapshot *self)
{
    S_zero_out(self);
    return self;
}

void
Snapshot_destroy(Snapshot *self)
{
    DECREF(self->entries);
    DECREF(self->path);
    SUPER_DESTROY(self, SNAPSHOT);
}

void
Snapshot_add_entry(Snapshot *self, const CharBuf *entry)
{
    Hash_Store(self->entries, (Obj*)entry, INCREF(&EMPTY));
}

bool_t
Snapshot_delete_entry(Snapshot *self, const CharBuf *entry)
{
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
Snapshot_num_entries(Snapshot *self) { return Hash_Get_Size(self->entries); }

void
Snapshot_set_path(Snapshot *self, const CharBuf *path)
{
    DECREF(self->path);
    self->path = path ? CB_Clone(path) : NULL;
}

CharBuf*
Snapshot_get_path(Snapshot *self) { return self->path; }

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

