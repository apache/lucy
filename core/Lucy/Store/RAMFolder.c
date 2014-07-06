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
static String*
S_fullpath(RAMFolder *self, String *path);

RAMFolder*
RAMFolder_new(String *path) {
    RAMFolder *self = (RAMFolder*)Class_Make_Obj(RAMFOLDER);
    return RAMFolder_init(self, path);
}

RAMFolder*
RAMFolder_init(RAMFolder *self, String *path) {
    Folder_init((Folder*)self, path);
    return self;
}

void
RAMFolder_Initialize_IMP(RAMFolder *self) {
    UNUSED_VAR(self);
}

bool
RAMFolder_Check_IMP(RAMFolder *self) {
    UNUSED_VAR(self);
    return true;
}

bool
RAMFolder_Local_MkDir_IMP(RAMFolder *self, String *name) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    if (Hash_Fetch(ivars->entries, (Obj*)name)) {
        Err_set_error(Err_new(Str_newf("Can't MkDir, '%o' already exists",
                                       name)));
        return false;
    }
    else {
        String *fullpath = S_fullpath(self, name);
        Hash_Store(ivars->entries, (Obj*)name,
                   (Obj*)RAMFolder_new(fullpath));
        DECREF(fullpath);
        return true;
    }
}

FileHandle*
RAMFolder_Local_Open_FileHandle_IMP(RAMFolder *self, String *name,
                                    uint32_t flags) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    RAMFileHandle *fh;
    String *fullpath = S_fullpath(self, name);
    RAMFile *file = (RAMFile*)Hash_Fetch(ivars->entries, (Obj*)name);
    bool can_create
        = (flags & (FH_WRITE_ONLY | FH_CREATE)) == (FH_WRITE_ONLY | FH_CREATE)
          ? true : false;

    // Make sure the filepath isn't a directory, and that it either exists
    // or we have permission to create it.
    if (file) {
        if (!RAMFile_Is_A(file, RAMFILE)) {
            Err_set_error(Err_new(Str_newf("Not a file: '%o'", fullpath)));
            DECREF(fullpath);
            return NULL;
        }
    }
    else if (!can_create) {
        Err_set_error(Err_new(Str_newf("File not found: '%o'", fullpath)));
        DECREF(fullpath);
        return NULL;
    }

    // Open the file and store it if it was just created.
    fh = RAMFH_open(fullpath, flags, file);
    if (fh) {
        if (!file) {
            file = RAMFH_Get_File(fh);
            Hash_Store(ivars->entries, (Obj*)name, INCREF(file));
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
RAMFolder_Local_Open_Dir_IMP(RAMFolder *self) {
    RAMDirHandle *dh = RAMDH_new(self);
    if (!dh) { ERR_ADD_FRAME(Err_get_error()); }
    return (DirHandle*)dh;
}

bool
RAMFolder_Local_Exists_IMP(RAMFolder *self, String *name) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    return !!Hash_Fetch(ivars->entries, (Obj*)name);
}

bool
RAMFolder_Local_Is_Directory_IMP(RAMFolder *self, String *name) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    Obj *entry = Hash_Fetch(ivars->entries, (Obj*)name);
    if (entry && Obj_Is_A(entry, FOLDER)) { return true; }
    return false;
}

#define OP_RENAME    1
#define OP_HARD_LINK 2

static bool
S_rename_or_hard_link(RAMFolder *self, String* from, String *to,
                      Folder *from_folder, Folder *to_folder,
                      String *from_name, String *to_name,
                      int op) {
    Obj       *elem              = NULL;
    RAMFolder *inner_from_folder = NULL;
    RAMFolder *inner_to_folder   = NULL;
    UNUSED_VAR(self);

    // Make sure the source and destination folders exist.
    if (!from_folder) {
        Err_set_error(Err_new(Str_newf("File not found: '%o'", from)));
        return false;
    }
    if (!to_folder) {
        Err_set_error(Err_new(Str_newf("Invalid file path (can't find dir): '%o'",
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
        Err_set_error(Err_new(Str_newf("Not a RAMFolder, but a '%o'",
                                       Obj_Get_Class_Name((Obj*)inner_from_folder))));
        return false;
    }
    if (!RAMFolder_Is_A(inner_to_folder, RAMFOLDER)) {
        Err_set_error(Err_new(Str_newf("Not a RAMFolder, but a '%o'",
                                       Obj_Get_Class_Name((Obj*)inner_to_folder))));
        return false;
    }

    // Find the original element.
    elem = Hash_Fetch(RAMFolder_IVARS(inner_from_folder)->entries,
                      (Obj*)from_name);
    if (!elem) {
        if (Folder_Is_A(from_folder, COMPOUNDFILEREADER)
            && Folder_Local_Exists(from_folder, from_name)
           ) {
            Err_set_error(Err_new(Str_newf("Source file '%o' is virtual",
                                           from)));
        }
        else {
            Err_set_error(Err_new(Str_newf("File not found: '%o'", from)));
        }
        return false;
    }

    // Execute the rename/hard-link.
    if (op == OP_RENAME) {
        Obj *existing = Hash_Fetch(RAMFolder_IVARS(inner_to_folder)->entries,
                                   (Obj*)to_name);
        if (existing) {
            bool conflict = false;

            // Return success fast if file is copied on top of itself.
            if (inner_from_folder == inner_to_folder
                && Str_Equals(from_name, (Obj*)to_name)
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
                Err_set_error(Err_new(Str_newf("Can't clobber a %o with a %o",
                                               Obj_Get_Class_Name(existing),
                                               Obj_Get_Class_Name(elem))));
                return false;
            }
        }

        // Perform the store first, then the delete. Inform Folder objects
        // about the relocation.
        Hash_Store(RAMFolder_IVARS(inner_to_folder)->entries,
                   (Obj*)to_name, INCREF(elem));
        DECREF(Hash_Delete(RAMFolder_IVARS(inner_from_folder)->entries,
                           (Obj*)from_name));
        if (Obj_Is_A(elem, FOLDER)) {
            String *newpath = S_fullpath(inner_to_folder, to_name);
            Folder_Set_Path((Folder*)elem, newpath);
            DECREF(newpath);
        }
    }
    else if (op == OP_HARD_LINK) {
        if (!Obj_Is_A(elem, RAMFILE)) {
            Err_set_error(Err_new(Str_newf("'%o' isn't a file, it's a %o",
                                           from, Obj_Get_Class_Name(elem))));
            return false;
        }
        else {
            Obj *existing
                = Hash_Fetch(RAMFolder_IVARS(inner_to_folder)->entries,
                             (Obj*)to_name);
            if (existing) {
                Err_set_error(Err_new(Str_newf("'%o' already exists", to)));
                return false;
            }
            else {
                Hash_Store(RAMFolder_IVARS(inner_to_folder)->entries,
                           (Obj*)to_name, INCREF(elem));
            }
        }
    }
    else {
        THROW(ERR, "Unexpected op: %i32", (int32_t)op);
    }

    return true;
}

bool
RAMFolder_Rename_IMP(RAMFolder *self, String* from,
                     String *to) {
    Folder *from_folder = RAMFolder_Enclosing_Folder(self, from);
    Folder *to_folder   = RAMFolder_Enclosing_Folder(self, to);
    String *from_name   = IxFileNames_local_part(from);
    String *to_name     = IxFileNames_local_part(to);
    bool result = S_rename_or_hard_link(self, from, to, from_folder, to_folder,
                                        from_name, to_name, OP_RENAME);
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    DECREF(to_name);
    DECREF(from_name);
    return result;
}

bool
RAMFolder_Hard_Link_IMP(RAMFolder *self, String *from,
                        String *to) {
    Folder *from_folder = RAMFolder_Enclosing_Folder(self, from);
    Folder *to_folder   = RAMFolder_Enclosing_Folder(self, to);
    String *from_name   = IxFileNames_local_part(from);
    String *to_name     = IxFileNames_local_part(to);
    bool result = S_rename_or_hard_link(self, from, to, from_folder, to_folder,
                                        from_name, to_name, OP_HARD_LINK);
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    DECREF(to_name);
    DECREF(from_name);
    return result;
}

bool
RAMFolder_Local_Delete_IMP(RAMFolder *self, String *name) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    Obj *entry = Hash_Fetch(ivars->entries, (Obj*)name);
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
            if (Hash_Get_Size(RAMFolder_IVARS(inner_folder)->entries)) {
                // Can't delete non-empty dir.
                return false;
            }
        }
        else {
            return false;
        }
        DECREF(Hash_Delete(ivars->entries, (Obj*)name));
        return true;
    }
    else {
        return false;
    }
}

Folder*
RAMFolder_Local_Find_Folder_IMP(RAMFolder *self, String *path) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    Folder *local_folder = (Folder*)Hash_Fetch(ivars->entries, (Obj*)path);
    if (local_folder && Folder_Is_A(local_folder, FOLDER)) {
        return local_folder;
    }
    return NULL;
}

void
RAMFolder_Close_IMP(RAMFolder *self) {
    UNUSED_VAR(self);
}

static String*
S_fullpath(RAMFolder *self, String *path) {
    RAMFolderIVARS *const ivars = RAMFolder_IVARS(self);
    if (Str_Get_Size(ivars->path)) {
        return Str_newf("%o/%o", ivars->path, path);
    }
    else {
        return Str_Clone(path);
    }
}


