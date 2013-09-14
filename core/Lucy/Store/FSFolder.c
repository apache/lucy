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

#define C_LUCY_FSFOLDER
#include "Lucy/Util/ToolSet.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef CHY_HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif

// For rmdir, (hard) link.
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

// For mkdir, rmdir.
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/CompoundFileReader.h"
#include "Lucy/Store/CompoundFileWriter.h"
#include "Lucy/Store/FSDirHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/IndexFileNames.h"

// Return a String containing a platform-specific absolute filepath.
static String*
S_fullpath(FSFolder *self, const String *path);

// Return true if the supplied path is a directory.
static bool
S_dir_ok(const String *path);

// Create a directory, or set Err_error and return false.
static bool
S_create_dir(const String *path);

// Return true unless the supplied path contains a slash.
static bool
S_is_local_entry(const String *path);

// Return true if the supplied path is absolute.
static bool
S_is_absolute(const String *path);

// Transform a possibly relative path into an absolute path.
static String*
S_absolutify(const String *path);

// Create a hard link.
static bool
S_hard_link(String *from_path, String *to_path);

FSFolder*
FSFolder_new(const String *path) {
    FSFolder *self = (FSFolder*)VTable_Make_Obj(FSFOLDER);
    return FSFolder_init(self, path);
}

FSFolder*
FSFolder_init(FSFolder *self, const String *path) {
    String *abs_path = S_absolutify(path);
    Folder_init((Folder*)self, abs_path);
    DECREF(abs_path);
    return self;
}

void
FSFolder_Initialize_IMP(FSFolder *self) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    if (!S_dir_ok(ivars->path)) {
        if (!S_create_dir(ivars->path)) {
            RETHROW(INCREF(Err_get_error()));
        }
    }
}

bool
FSFolder_Check_IMP(FSFolder *self) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    return S_dir_ok(ivars->path);
}

FileHandle*
FSFolder_Local_Open_FileHandle_IMP(FSFolder *self, const String *name,
                                   uint32_t flags) {
    String       *fullpath = S_fullpath(self, name);
    FSFileHandle *fh = FSFH_open(fullpath, flags);
    if (!fh) { ERR_ADD_FRAME(Err_get_error()); }
    DECREF(fullpath);
    return (FileHandle*)fh;
}

bool
FSFolder_Local_MkDir_IMP(FSFolder *self, const String *name) {
    String *dir = S_fullpath(self, name);
    bool result = S_create_dir(dir);
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    DECREF(dir);
    return result;
}

DirHandle*
FSFolder_Local_Open_Dir_IMP(FSFolder *self) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    DirHandle *dh = (DirHandle*)FSDH_open(ivars->path);
    if (!dh) { ERR_ADD_FRAME(Err_get_error()); }
    return dh;
}

bool
FSFolder_Local_Exists_IMP(FSFolder *self, const String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    if (Hash_Fetch(ivars->entries, (Obj*)name)) {
        return true;
    }
    else if (!S_is_local_entry(name)) {
        return false;
    }
    else {
        struct stat stat_buf;
        String *fullpath = S_fullpath(self, name);
        bool retval = false;
        if (stat((char*)Str_Get_Ptr8(fullpath), &stat_buf) != -1) {
            retval = true;
        }
        DECREF(fullpath);
        return retval;
    }
}

bool
FSFolder_Local_Is_Directory_IMP(FSFolder *self, const String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);

    // Check for a cached object, then fall back to a system call.
    Obj *elem = Hash_Fetch(ivars->entries, (Obj*)name);
    if (elem && Obj_Is_A(elem, FOLDER)) {
        return true;
    }
    else {
        String *fullpath = S_fullpath(self, name);
        bool result = S_dir_ok(fullpath);
        DECREF(fullpath);
        return result;
    }
}

bool
FSFolder_Rename_IMP(FSFolder *self, const String* from, const String *to) {
    String *from_path = S_fullpath(self, from);
    String *to_path   = S_fullpath(self, to);
    bool    retval    = !rename((char*)Str_Get_Ptr8(from_path),
                                 (char*)Str_Get_Ptr8(to_path));
    if (!retval) {
        Err_set_error(Err_new(Str_newf("rename from '%o' to '%o' failed: %s",
                                       from_path, to_path, strerror(errno))));
    }
    DECREF(from_path);
    DECREF(to_path);
    return retval;
}

bool
FSFolder_Hard_Link_IMP(FSFolder *self, const String *from,
                       const String *to) {
    String *from_path = S_fullpath(self, from);
    String *to_path   = S_fullpath(self, to);
    bool    retval    = S_hard_link(from_path, to_path);
    DECREF(from_path);
    DECREF(to_path);
    return retval;
}

bool
FSFolder_Local_Delete_IMP(FSFolder *self, const String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);

    String *fullpath = S_fullpath(self, name);
    char   *path_ptr = (char*)Str_Get_Ptr8(fullpath);
#ifdef CHY_REMOVE_ZAPS_DIRS
    bool result = !remove(path_ptr);
#else
    bool result = !rmdir(path_ptr) || !remove(path_ptr);
#endif
    DECREF(Hash_Delete(ivars->entries, (Obj*)name));
    DECREF(fullpath);
    return result;
}

void
FSFolder_Close_IMP(FSFolder *self) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    Hash_Clear(ivars->entries);
}

Folder*
FSFolder_Local_Find_Folder_IMP(FSFolder *self, const String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);

    Folder *subfolder = NULL;
    if (!name || !Str_Get_Size(name)) {
        // No entity can be identified by NULL or empty string.
        return NULL;
    }
    else if (!S_is_local_entry(name)) {
        return NULL;
    }
    else if (Str_Starts_With_Str(name, ".", 1)) {
        // Don't allow access outside of the main dir.
        return NULL;
    }
    else if (NULL != (subfolder = (Folder*)Hash_Fetch(ivars->entries, (Obj*)name))) {
        if (Folder_Is_A(subfolder, FOLDER)) {
            return subfolder;
        }
        else {
            return NULL;
        }
    }

    String *fullpath = S_fullpath(self, name);
    if (S_dir_ok(fullpath)) {
        subfolder = (Folder*)FSFolder_new(fullpath);
        if (!subfolder) {
            DECREF(fullpath);
            THROW(ERR, "Failed to open FSFolder at '%o'", fullpath);
        }
        // Try to open a CompoundFileReader. On failure, just use the
        // existing folder.
        String *cfmeta_file = (String*)SSTR_WRAP_STR("cfmeta.json", 11);
        if (Folder_Local_Exists(subfolder, cfmeta_file)) {
            CompoundFileReader *cf_reader = CFReader_open(subfolder);
            if (cf_reader) {
                DECREF(subfolder);
                subfolder = (Folder*)cf_reader;
            }
        }
        Hash_Store(ivars->entries, (Obj*)name, (Obj*)subfolder);
    }
    DECREF(fullpath);

    return subfolder;
}

static String*
S_fullpath(FSFolder *self, const String *path) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    String *fullpath = Str_newf("%o%s%o", ivars->path, DIR_SEP, path);
    String *retval;
    if (DIR_SEP[0] != '/') {
        retval = Str_Swap_Chars(fullpath, '/', DIR_SEP[0]);
        DECREF(fullpath);
    }
    else {
        retval = fullpath;
    }
    return retval;
}

static bool
S_dir_ok(const String *path) {
    struct stat stat_buf;
    if (stat((char*)Str_Get_Ptr8(path), &stat_buf) != -1) {
        if (stat_buf.st_mode & S_IFDIR) { return true; }
    }
    return false;
}

static bool
S_create_dir(const String *path) {
    if (-1 == chy_makedir((char*)Str_Get_Ptr8(path), 0777)) {
        Err_set_error(Err_new(Str_newf("Couldn't create directory '%o': %s",
                                       path, strerror(errno))));
        return false;
    }
    return true;
}

static bool
S_is_local_entry(const String *path) {
    return Str_Find_Str(path, "/", 1) == -1;
}

/***************************************************************************/

#if (defined(CHY_HAS_WINDOWS_H) && !defined(__CYGWIN__))

// Windows.h defines INCREF and DECREF, so we include it only at the end of
// this file and undef those symbols.
#undef INCREF
#undef DECREF

#include <windows.h>

static bool
S_is_absolute(const String *path) {
    uint32_t code_point = Str_Code_Point_At(path, 0);

    if (isalpha(code_point)) {
        code_point = Str_Code_Point_At(path, 1);
        if (code_point != ':') { return false; }
        code_point = Str_Code_Point_At(path, 2);
    }

    return code_point == '\\' || code_point == '/';
}

static String*
S_absolutify(const String *path) {
    if (S_is_absolute(path)) { return Str_Clone(path); }

    DWORD  cwd_len = GetCurrentDirectory(0, NULL);
    char  *cwd     = (char*)MALLOCATE(cwd_len);
    DWORD  res     = GetCurrentDirectory(cwd_len, cwd);
    if (res == 0 || res > cwd_len) {
        THROW(ERR, "GetCurrentDirectory failed");
    }
    String *abs_path = Str_newf("%s\\%o", cwd, path);
    FREEMEM(cwd);

    return abs_path;
}

static bool
S_hard_link(String *from_path, String *to_path) {
    char *from8 = (char*)Str_Get_Ptr8(from_path);
    char *to8   = (char*)Str_Get_Ptr8(to_path);

    if (CreateHardLink(to8, from8, NULL)) {
        return true;
    }
    else {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(Str_newf("CreateHardLink for new file '%o' from '%o' failed: %s",
                                       to_path, from_path, win_error)));
        FREEMEM(win_error);
        return false;
    }
}

#elif (defined(CHY_HAS_UNISTD_H))

static bool
S_is_absolute(const String *path) {
    return Str_Starts_With_Str(path, DIR_SEP, 1);
}

static String*
S_absolutify(const String *path) {
    if (S_is_absolute(path)) { return Str_Clone(path); }

    char *cwd = getcwd(NULL, 0);
    if (!cwd) { THROW(ERR, "getcwd failed"); }
    String *abs_path = Str_newf("%s" DIR_SEP "%o", cwd, path);
    free(cwd);

    return abs_path;
}

static bool
S_hard_link(String *from_path, String *to_path) {
    char *from8 = (char*)Str_Get_Ptr8(from_path);
    char *to8   = (char*)Str_Get_Ptr8(to_path);

    if (-1 == link(from8, to8)) {
        Err_set_error(Err_new(Str_newf("hard link for new file '%o' from '%o' failed: %s",
                                       to_path, from_path, strerror(errno))));
        return false;
    }
    else {
        return true;
    }
}

#else
  #error "Need either windows.h or unistd.h"
#endif /* CHY_HAS_UNISTD_H vs. CHY_HAS_WINDOWS_H */


