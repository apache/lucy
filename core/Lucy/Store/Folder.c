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

#define C_LUCY_FOLDER
#include "Lucy/Util/ToolSet.h"
#include <ctype.h>
#include <limits.h>

#include "Lucy/Store/Folder.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/IndexFileNames.h"

Folder*
Folder_init(Folder *self, const CharBuf *path) {
    FolderIVARS *const ivars = Folder_IVARS(self);

    // Init.
    ivars->entries = Hash_new(16);

    // Copy.
    if (path == NULL) {
        ivars->path = CB_new_from_trusted_utf8("", 0);
    }
    else {
        // Copy path, strip trailing slash or equivalent.
        if (CB_Ends_With_Str(path, DIR_SEP, strlen(DIR_SEP))) {
            ivars->path = CB_SubString(path, 0, CB_Length(path) - 1);
        }
        else {
            ivars->path = CB_Clone(path);
        }
    }

    ABSTRACT_CLASS_CHECK(self, FOLDER);
    return self;
}

void
Folder_Destroy_IMP(Folder *self) {
    FolderIVARS *const ivars = Folder_IVARS(self);
    DECREF(ivars->path);
    DECREF(ivars->entries);
    SUPER_DESTROY(self, FOLDER);
}

InStream*
Folder_Open_In_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    InStream *instream = NULL;

    if (enclosing_folder) {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        instream = Folder_Local_Open_In(enclosing_folder, (CharBuf*)name);
        if (!instream) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }
    else {
        Err_set_error(Err_new(CB_newf("Invalid path: '%o'", path)));
    }

    return instream;
}

/* This method exists as a hook for CompoundFileReader to override; it is
 * necessary because calling CFReader_Local_Open_FileHandle() won't find
 * virtual files.  No other class should need to override it. */
InStream*
Folder_Local_Open_In_IMP(Folder *self, const CharBuf *name) {
    FileHandle *fh = Folder_Local_Open_FileHandle(self, name, FH_READ_ONLY);
    InStream *instream = NULL;
    if (fh) {
        instream = InStream_open((Obj*)fh);
        DECREF(fh);
        if (!instream) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }
    else {
        ERR_ADD_FRAME(Err_get_error());
    }
    return instream;
}

OutStream*
Folder_Open_Out_IMP(Folder *self, const CharBuf *path) {
    const uint32_t flags = FH_WRITE_ONLY | FH_CREATE | FH_EXCLUSIVE;
    FileHandle *fh = Folder_Open_FileHandle(self, path, flags);
    OutStream *outstream = NULL;
    if (fh) {
        outstream = OutStream_open((Obj*)fh);
        DECREF(fh);
        if (!outstream) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }
    else {
        ERR_ADD_FRAME(Err_get_error());
    }
    return outstream;
}

FileHandle*
Folder_Open_FileHandle_IMP(Folder *self, const CharBuf *path,
                           uint32_t flags) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    FileHandle *fh = NULL;

    if (enclosing_folder) {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        fh = Folder_Local_Open_FileHandle(enclosing_folder,
                                          (CharBuf*)name, flags);
        if (!fh) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }
    else {
        Err_set_error(Err_new(CB_newf("Invalid path: '%o'", path)));
    }

    return fh;
}

bool
Folder_Delete_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    if (enclosing_folder) {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        bool result = Folder_Local_Delete(enclosing_folder, (CharBuf*)name);
        return result;
    }
    else {
        return false;
    }
}

bool
Folder_Delete_Tree_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);

    // Don't allow Folder to delete itself.
    if (!path || !CB_Get_Size(path)) { return false; }

    if (enclosing_folder) {
        StackString *local = IxFileNames_local_part(path, SStr_BLANK());
        if (Folder_Local_Is_Directory(enclosing_folder, (CharBuf*)local)) {
            Folder *inner_folder
                = Folder_Local_Find_Folder(enclosing_folder, (CharBuf*)local);
            DirHandle *dh = Folder_Local_Open_Dir(inner_folder);
            if (dh) {
                VArray *files = VA_new(20);
                VArray *dirs  = VA_new(20);
                while (DH_Next(dh)) {
                    CharBuf *entry = DH_Get_Entry(dh);
                    VA_Push(files, (Obj*)CB_Clone(entry));
                    if (DH_Entry_Is_Dir(dh) && !DH_Entry_Is_Symlink(dh)) {
                        VA_Push(dirs, (Obj*)CB_Clone(entry));
                    }
                    DECREF(entry);
                }
                for (uint32_t i = 0, max = VA_Get_Size(dirs); i < max; i++) {
                    CharBuf *name = (CharBuf*)VA_Fetch(files, i);
                    bool success = Folder_Delete_Tree(inner_folder, name);
                    if (!success && Folder_Local_Exists(inner_folder, name)) {
                        break;
                    }
                }
                for (uint32_t i = 0, max = VA_Get_Size(files); i < max; i++) {
                    CharBuf *name = (CharBuf*)VA_Fetch(files, i);
                    bool success = Folder_Local_Delete(inner_folder, name);
                    if (!success && Folder_Local_Exists(inner_folder, name)) {
                        break;
                    }
                }
                DECREF(dirs);
                DECREF(files);
                DECREF(dh);
            }
        }
        return Folder_Local_Delete(enclosing_folder, (CharBuf*)local);
    }
    else {
        // Return failure if the entry wasn't there in the first place.
        return false;
    }
}

static bool
S_is_updir(CharBuf *path) {
    if (CB_Equals_Str(path, ".", 1) || CB_Equals_Str(path, "..", 2)) {
        return true;
    }
    else {
        return false;
    }
}

static void
S_add_to_file_list(Folder *self, VArray *list, CharBuf *dir,
                   const CharBuf *path) {
    DirHandle *dh = Folder_Open_Dir(self, dir);

    if (!dh) {
        RETHROW(INCREF(Err_get_error()));
    }

    while (DH_Next(dh)) { // Updates entry
        CharBuf *entry = DH_Get_Entry(dh);
        if (!S_is_updir(entry)) {
            CharBuf *relpath = path && CB_Get_Size(path)
                               ? CB_newf("%o/%o", path, entry)
                               : CB_Clone(entry);
            if (VA_Get_Size(list) == VA_Get_Capacity(list)) {
                VA_Grow(list, VA_Get_Size(list) * 2);
            }
            VA_Push(list, (Obj*)relpath);

            if (DH_Entry_Is_Dir(dh) && !DH_Entry_Is_Symlink(dh)) {
                CharBuf *subdir = CB_Get_Size(dir)
                                  ? CB_newf("%o/%o", dir, entry)
                                  : CB_Clone(entry);
                S_add_to_file_list(self, list, subdir, relpath); // recurse
                DECREF(subdir);
            }
        }
        DECREF(entry);
    }

    if (!DH_Close(dh)) {
        RETHROW(INCREF(Err_get_error()));
    }
    DECREF(dh);
}

DirHandle*
Folder_Open_Dir_IMP(Folder *self, const CharBuf *path) {
    DirHandle *dh = NULL;
    Folder *folder;
    if (path) {
        folder = Folder_Find_Folder(self, path);
    }
    else {
        StackString *empty = SStr_BLANK();
        folder = Folder_Find_Folder(self, (CharBuf*)empty);
    }
    if (!folder) {
        Err_set_error(Err_new(CB_newf("Invalid path: '%o'", path)));
    }
    else {
        dh = Folder_Local_Open_Dir(folder);
        if (!dh) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }
    return dh;
}

bool
Folder_MkDir_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    bool result = false;

    if (!CB_Get_Size(path)) {
        Err_set_error(Err_new(CB_newf("Invalid path: '%o'", path)));
    }
    else if (!enclosing_folder) {
        Err_set_error(Err_new(CB_newf("Can't recursively create dir %o",
                                      path)));
    }
    else {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        result = Folder_Local_MkDir(enclosing_folder, (CharBuf*)name);
        if (!result) {
            ERR_ADD_FRAME(Err_get_error());
        }
    }

    return result;
}

bool
Folder_Exists_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    bool retval = false;
    if (enclosing_folder) {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        if (Folder_Local_Exists(enclosing_folder, (CharBuf*)name)) {
            retval = true;
        }
    }
    return retval;
}

bool
Folder_Is_Directory_IMP(Folder *self, const CharBuf *path) {
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    bool retval = false;
    if (enclosing_folder) {
        StackString *name = IxFileNames_local_part(path, SStr_BLANK());
        if (Folder_Local_Is_Directory(enclosing_folder, (CharBuf*)name)) {
            retval = true;
        }
    }
    return retval;
}

VArray*
Folder_List_IMP(Folder *self, const CharBuf *path) {
    Folder *local_folder = Folder_Find_Folder(self, path);
    VArray *list = NULL;
    DirHandle *dh = Folder_Local_Open_Dir(local_folder);
    if (dh) {
        list = VA_new(32);
        while (DH_Next(dh)) {
            CharBuf *entry = DH_Get_Entry(dh);
            VA_Push(list, (Obj*)CB_Clone(entry));
            DECREF(entry);
        }
        DECREF(dh);
    }
    else {
        ERR_ADD_FRAME(Err_get_error());
    }
    return list;
}

VArray*
Folder_List_R_IMP(Folder *self, const CharBuf *path) {
    Folder *local_folder = Folder_Find_Folder(self, path);
    VArray *list =  VA_new(0);
    if (local_folder) {
        CharBuf *dir    = CB_new(20);
        S_add_to_file_list(local_folder, list, dir, path);
        DECREF(dir);
    }
    return list;
}

ByteBuf*
Folder_Slurp_File_IMP(Folder *self, const CharBuf *path) {
    InStream *instream = Folder_Open_In(self, path);
    ByteBuf  *retval   = NULL;

    if (!instream) {
        RETHROW(INCREF(Err_get_error()));
    }
    else {
        uint64_t length = InStream_Length(instream);

        if (length >= SIZE_MAX) {
            InStream_Close(instream);
            DECREF(instream);
            THROW(ERR, "File %o is too big to slurp (%u64 bytes)", path,
                  length);
        }
        else {
            size_t size = (size_t)length;
            char *ptr = (char*)MALLOCATE((size_t)size + 1);
            InStream_Read_Bytes(instream, ptr, size);
            ptr[size] = '\0';
            retval = BB_new_steal_bytes(ptr, size, size + 1);
            InStream_Close(instream);
            DECREF(instream);
        }
    }

    return retval;
}

CharBuf*
Folder_Get_Path_IMP(Folder *self) {
    return Folder_IVARS(self)->path;
}

void
Folder_Set_Path_IMP(Folder *self, const CharBuf *path) {
    FolderIVARS *const ivars = Folder_IVARS(self);
    DECREF(ivars->path);
    ivars->path = CB_Clone(path);
}

void
Folder_Consolidate_IMP(Folder *self, const CharBuf *path) {
    Folder *folder = Folder_Find_Folder(self, path);
    Folder *enclosing_folder = Folder_Enclosing_Folder(self, path);
    if (!folder) {
        THROW(ERR, "Can't consolidate %o", path);
    }
    else if (Folder_Is_A(folder, COMPOUNDFILEREADER)) {
        THROW(ERR, "Can't consolidate %o twice", path);
    }
    else {
        CompoundFileWriter *cf_writer = CFWriter_new(folder);
        CFWriter_Consolidate(cf_writer);
        DECREF(cf_writer);
        if (CB_Get_Size(path)) {
            StackString *name = IxFileNames_local_part(path, SStr_BLANK());
            CompoundFileReader *cf_reader = CFReader_open(folder);
            if (!cf_reader) { RETHROW(INCREF(Err_get_error())); }
            Hash *entries = Folder_IVARS(enclosing_folder)->entries;
            Hash_Store(entries, (Obj*)name,
                       (Obj*)cf_reader);
        }
    }
}

static Folder*
S_enclosing_folder(Folder *self, StackString *path) {
    size_t path_component_len = 0;
    uint32_t code_point;

    // Strip trailing slash.
    if (SStr_Code_Point_From(path, 0) == '/') { SStr_Chop(path, 1); }

    // Find first component of the file path.
    StackString *scratch        = SSTR_WRAP((CharBuf*)path);
    StackString *path_component = SSTR_WRAP((CharBuf*)path);
    while (0 != (code_point = SStr_Nibble(scratch))) {
        if (code_point == '/') {
            SStr_Truncate(path_component, path_component_len);
            SStr_Nip(path, path_component_len + 1);
            break;
        }
        path_component_len++;
    }

    // If we've eaten up the entire filepath, self is enclosing folder.
    if (SStr_Get_Size(scratch) == 0) { return self; }

    Folder *local_folder
        = Folder_Local_Find_Folder(self, (CharBuf*)path_component);
    if (!local_folder) {
        /* This element of the filepath doesn't exist, or it's not a
         * directory.  However, there are filepath characters left over,
         * implying that this component ought to be a directory -- so the
         * original file path is invalid. */
        return NULL;
    }

    // This file path component is a folder.  Recurse into it.
    return S_enclosing_folder(local_folder, path);
}

Folder*
Folder_Enclosing_Folder_IMP(Folder *self, const CharBuf *path) {
    StackString *scratch = SSTR_WRAP(path);
    return S_enclosing_folder(self, scratch);
}

Folder*
Folder_Find_Folder_IMP(Folder *self, const CharBuf *path) {
    if (!path || !CB_Get_Size(path)) {
        return self;
    }
    else {
        StackString *scratch = SSTR_WRAP(path);
        Folder *enclosing_folder = S_enclosing_folder(self, scratch);
        if (!enclosing_folder) {
            return NULL;
        }
        else {
            return Folder_Local_Find_Folder(enclosing_folder,
                                            (CharBuf*)scratch);
        }
    }
}


