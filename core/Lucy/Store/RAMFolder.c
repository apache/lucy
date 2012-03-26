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

#define C_LUCY_RAMFOLDER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMDirHandle.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Util/IndexFileNames.h"

// Return the concatenation of the Folder's path and the supplied path.
static CharBuf*
S_fullpath(RAMFolder *self, const CharBuf *path);

RAMFolder*
RAMFolder_new(const CharBuf *path) {
    RAMFolder *self = (RAMFolder*)VTable_Make_Obj(RAMFOLDER);
    return RAMFolder_init(self, path);
}

RAMFolder*
RAMFolder_init(RAMFolder *self, const CharBuf *path) {
    Folder_init((Folder*)self, path);
    return self;
}

void
RAMFolder_initialize(RAMFolder *self) {
    UNUSED_VAR(self);
}

bool_t
RAMFolder_check(RAMFolder *self) {
    UNUSED_VAR(self);
    return true;
}

bool_t
RAMFolder_local_mkdir(RAMFolder *self, const CharBuf *name) {
    if (Hash_Fetch(self->entries, (Obj*)name)) {
        Err_set_error(Err_new(CB_newf("Can't MkDir, '%o' already exists",
                                      name)));
        return false;
    }
    else {
        CharBuf *fullpath = S_fullpath(self, name);
        Hash_Store(self->entries, (Obj*)name,
                   (Obj*)RAMFolder_new(fullpath));
        DECREF(fullpath);
        return true;
    }
}

FileHandle*
RAMFolder_local_open_filehandle(RAMFolder *self, const CharBuf *name,
                                uint32_t flags) {
    RAMFileHandle *fh;
    CharBuf *fullpath = S_fullpath(self, name);
    RAMFile *file = (RAMFile*)Hash_Fetch(self->entries, (Obj*)name);
    bool_t can_create
        = (flags & (FH_WRITE_ONLY | FH_CREATE)) == (FH_WRITE_ONLY | FH_CREATE)
          ? true : false;

    // Make sure the filepath isn't a directory, and that it either exists
    // or we have permission to create it.
    if (file) {
        if (!RAMFile_Is_A(file, RAMFILE)) {
            Err_set_error(Err_new(CB_newf("Not a file: '%o'", fullpath)));
            DECREF(fullpath);
            return NULL;
        }
    }
    else if (!can_create) {
        Err_set_error(Err_new(CB_newf("File not found: '%o'", fullpath)));
        DECREF(fullpath);
        return NULL;
    }

    // Open the file and store it if it was just created.
    fh = RAMFH_open(fullpath, flags, file);
    if (fh) {
        if (!file) {
            file = RAMFH_Get_File(fh);
            Hash_Store(self->entries, (Obj*)name, INCREF(file));
        }
    }
    else {
        Err *error = Err_get_error();
        ERR_ADD_FRAME(error);
    }

    DECREF(fullpath);

    return (FileHandle*)fh;
}

DirHandle*
RAMFolder_local_open_dir(RAMFolder *self) {
    RAMDirHandle *dh = RAMDH_new(self);
    if (!dh) { ERR_ADD_FRAME(Err_get_error()); }
    return (DirHandle*)dh;
}

bool_t
RAMFolder_local_exists(RAMFolder *self, const CharBuf *name) {
    return !!Hash_Fetch(self->entries, (Obj*)name);
}

bool_t
RAMFolder_local_is_directory(RAMFolder *self, const CharBuf *name) {
    Obj *entry = Hash_Fetch(self->entries, (Obj*)name);
    if (entry && Obj_Is_A(entry, FOLDER)) { return true; }
    return false;
}

#define OP_RENAME    1
#define OP_HARD_LINK 2

static bool_t
S_rename_or_hard_link(RAMFolder *self, const CharBuf* from, const CharBuf *to,
                      Folder *from_folder, Folder *to_folder,
                      ZombieCharBuf *from_name, ZombieCharBuf *to_name,
                      int op) {
    Obj       *elem              = NULL;
    RAMFolder *inner_from_folder = NULL;
    RAMFolder *inner_to_folder   = NULL;
    UNUSED_VAR(self);

    // Make sure the source and destination folders exist.
    if (!from_folder) {
        Err_set_error(Err_new(CB_newf("File not found: '%o'", from)));
        return false;
    }
    if (!to_folder) {
        Err_set_error(Err_new(CB_newf("Invalid file path (can't find dir): '%o'",
                                      to)));
        return false;
    }

    // Extract RAMFolders from compound reader wrappers, if necessary.
    if (Folder_Is_A(from_folder, COMPOUNDFILEREADER)) {
        inner_from_folder = (RAMFolder*)CFReader_Get_Real_Folder(
                                (CompoundFileReader*)from_folder);
    }
    else {
        inner_from_folder = (RAMFolder*)from_folder;
    }
    if (Folder_Is_A(to_folder, COMPOUNDFILEREADER)) {
        inner_to_folder = (RAMFolder*)CFReader_Get_Real_Folder(
                              (CompoundFileReader*)to_folder);
    }
    else {
        inner_to_folder = (RAMFolder*)to_folder;
    }
    if (!RAMFolder_Is_A(inner_from_folder, RAMFOLDER)) {
        Err_set_error(Err_new(CB_newf("Not a RAMFolder, but a '%o'",
                                      Obj_Get_Class_Name((Obj*)inner_from_folder))));
        return false;
    }
    if (!RAMFolder_Is_A(inner_to_folder, RAMFOLDER)) {
        Err_set_error(Err_new(CB_newf("Not a RAMFolder, but a '%o'",
                                      Obj_Get_Class_Name((Obj*)inner_to_folder))));
        return false;
    }

    // Find the original element.
    elem = Hash_Fetch(inner_from_folder->entries, (Obj*)from_name);
    if (!elem) {
        if (Folder_Is_A(from_folder, COMPOUNDFILEREADER)
            && Folder_Local_Exists(from_folder, (CharBuf*)from_name)
           ) {
            Err_set_error(Err_new(CB_newf("Source file '%o' is virtual",
                                          from)));
        }
        else {
            Err_set_error(Err_new(CB_newf("File not found: '%o'", from)));
        }
        return false;
    }

    // Execute the rename/hard-link.
    if (op == OP_RENAME) {
        Obj *existing = Hash_Fetch(inner_to_folder->entries, (Obj*)to_name);
        if (existing) {
            bool_t conflict = false;

            // Return success fast if file is copied on top of itself.
            if (inner_from_folder == inner_to_folder
                && ZCB_Equals(from_name, (Obj*)to_name)
               ) {
                return true;
            }

            // Don't allow clobbering of different entry type.
            if (Obj_Is_A(elem, RAMFILE)) {
                if (!Obj_Is_A(existing, RAMFILE)) {
                    conflict = true;
                }
            }
            else if (Obj_Is_A(elem, FOLDER)) {
                if (!Obj_Is_A(existing, FOLDER)) {
                    conflict = true;
                }
            }
            if (conflict) {
                Err_set_error(Err_new(CB_newf("Can't clobber a %o with a %o",
                                              Obj_Get_Class_Name(existing),
                                              Obj_Get_Class_Name(elem))));
                return false;
            }
        }

        // Perform the store first, then the delete. Inform Folder objects
        // about the relocation.
        Hash_Store(inner_to_folder->entries, (Obj*)to_name, INCREF(elem));
        DECREF(Hash_Delete(inner_from_folder->entries, (Obj*)from_name));
        if (Obj_Is_A(elem, FOLDER)) {
            CharBuf *newpath = S_fullpath(inner_to_folder, (CharBuf*)to_name);
            Folder_Set_Path((Folder*)elem, newpath);
            DECREF(newpath);
        }
    }
    else if (op == OP_HARD_LINK) {
        if (!Obj_Is_A(elem, RAMFILE)) {
            Err_set_error(Err_new(CB_newf("'%o' isn't a file, it's a %o",
                                          from, Obj_Get_Class_Name(elem))));
            return false;
        }
        else {
            Obj *existing
                = Hash_Fetch(inner_to_folder->entries, (Obj*)to_name);
            if (existing) {
                Err_set_error(Err_new(CB_newf("'%o' already exists", to)));
                return false;
            }
            else {
                Hash_Store(inner_to_folder->entries, (Obj*)to_name,
                           INCREF(elem));
            }
        }
    }
    else {
        THROW(ERR, "Unexpected op: %i32", (int32_t)op);
    }

    return true;
}

bool_t
RAMFolder_rename(RAMFolder *self, const CharBuf* from, const CharBuf *to) {
    Folder        *from_folder = RAMFolder_Enclosing_Folder(self, from);
    Folder        *to_folder   = RAMFolder_Enclosing_Folder(self, to);
    ZombieCharBuf *from_name   = IxFileNames_local_part(from, ZCB_BLANK());
    ZombieCharBuf *to_name     = IxFileNames_local_part(to, ZCB_BLANK());
    bool_t result = S_rename_or_hard_link(self, from, to, from_folder,
                                          to_folder, from_name, to_name,
                                          OP_RENAME);
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    return result;
}

bool_t
RAMFolder_hard_link(RAMFolder *self, const CharBuf *from, const CharBuf *to) {
    Folder        *from_folder = RAMFolder_Enclosing_Folder(self, from);
    Folder        *to_folder   = RAMFolder_Enclosing_Folder(self, to);
    ZombieCharBuf *from_name   = IxFileNames_local_part(from, ZCB_BLANK());
    ZombieCharBuf *to_name     = IxFileNames_local_part(to, ZCB_BLANK());
    bool_t result = S_rename_or_hard_link(self, from, to, from_folder,
                                          to_folder, from_name, to_name,
                                          OP_HARD_LINK);
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    return result;
}

bool_t
RAMFolder_local_delete(RAMFolder *self, const CharBuf *name) {
    Obj *entry = Hash_Fetch(self->entries, (Obj*)name);
    if (entry) {
        if (Obj_Is_A(entry, RAMFILE)) {
            ;
        }
        else if (Obj_Is_A(entry, FOLDER)) {
            RAMFolder *inner_folder;
            if (Obj_Is_A(entry, COMPOUNDFILEREADER)) {
                inner_folder = (RAMFolder*)CERTIFY(
                                   CFReader_Get_Real_Folder((CompoundFileReader*)entry),
                                   RAMFOLDER);
            }
            else {
                inner_folder = (RAMFolder*)CERTIFY(entry, RAMFOLDER);
            }
            if (Hash_Get_Size(inner_folder->entries)) {
                // Can't delete non-empty dir.
                return false;
            }
        }
        else {
            return false;
        }
        DECREF(Hash_Delete(self->entries, (Obj*)name));
        return true;
    }
    else {
        return false;
    }
}

Folder*
RAMFolder_local_find_folder(RAMFolder *self, const CharBuf *path) {
    Folder *local_folder = (Folder*)Hash_Fetch(self->entries, (Obj*)path);
    if (local_folder && Folder_Is_A(local_folder, FOLDER)) {
        return local_folder;
    }
    return NULL;
}

void
RAMFolder_close(RAMFolder *self) {
    UNUSED_VAR(self);
}

static CharBuf*
S_fullpath(RAMFolder *self, const CharBuf *path) {
    if (CB_Get_Size(self->path)) {
        return CB_newf("%o/%o", self->path, path);
    }
    else {
        return CB_Clone(path);
    }
}


