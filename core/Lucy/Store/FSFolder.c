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

#include "charmony.h"

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
S_fullpath(FSFolder *self, String *path);

// Return a String containing a platform-specific absolute filepath.
static char*
S_fullpath_ptr(FSFolder *self, String *path);

// Return true if the supplied path is a directory.
static bool
S_dir_ok(String *path);

// Create a directory, or set Err_error and return false.
static bool
S_create_dir(String *path);

// Return true unless the supplied path contains a slash.
static bool
S_is_local_entry(String *path);

// Return true if the supplied path is absolute.
static bool
S_is_absolute(String *path);

// Transform a possibly relative path into an absolute path.
static String*
S_absolutify(String *path);

// Create a hard link.
static bool
S_hard_link(char *from_path, char *to_path);

FSFolder*
FSFolder_new(String *path) {
    FSFolder *self = (FSFolder*)Class_Make_Obj(FSFOLDER);
    return FSFolder_init(self, path);
}

FSFolder*
FSFolder_init(FSFolder *self, String *path) {
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
FSFolder_Local_Open_FileHandle_IMP(FSFolder *self, String *name,
                                   uint32_t flags) {
    String       *fullpath = S_fullpath(self, name);
    FSFileHandle *fh = FSFH_open(fullpath, flags);
    if (!fh) { ERR_ADD_FRAME(Err_get_error()); }
    DECREF(fullpath);
    return (FileHandle*)fh;
}

bool
FSFolder_Local_MkDir_IMP(FSFolder *self, String *name) {
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
FSFolder_Local_Exists_IMP(FSFolder *self, String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    if (Hash_Fetch(ivars->entries, (Obj*)name)) {
        return true;
    }
    else if (!S_is_local_entry(name)) {
        return false;
    }
    else {
        struct stat stat_buf;
        char *fullpath_ptr = S_fullpath_ptr(self, name);
        bool retval = false;
        if (stat(fullpath_ptr, &stat_buf) != -1) {
            retval = true;
        }
        FREEMEM(fullpath_ptr);
        return retval;
    }
}

bool
FSFolder_Local_Is_Directory_IMP(FSFolder *self, String *name) {
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
FSFolder_Rename_IMP(FSFolder *self, String* from, String *to) {
    char *from_path = S_fullpath_ptr(self, from);
    char *to_path   = S_fullpath_ptr(self, to);
    bool  retval    = !rename(from_path, to_path);
    if (!retval) {
        Err_set_error(Err_new(Str_newf("rename from '%s' to '%s' failed: %s",
                                       from_path, to_path, strerror(errno))));
    }
    FREEMEM(from_path);
    FREEMEM(to_path);
    return retval;
}

bool
FSFolder_Hard_Link_IMP(FSFolder *self, String *from,
                       String *to) {
    char *from_path_ptr = S_fullpath_ptr(self, from);
    char *to_path_ptr   = S_fullpath_ptr(self, to);
    bool  retval        = S_hard_link(from_path_ptr, to_path_ptr);
    FREEMEM(from_path_ptr);
    FREEMEM(to_path_ptr);
    return retval;
}

bool
FSFolder_Local_Delete_IMP(FSFolder *self, String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);

    char *path_ptr = S_fullpath_ptr(self, name);
#ifdef CHY_REMOVE_ZAPS_DIRS
    bool result = !remove(path_ptr);
#else
    bool result = !rmdir(path_ptr) || !remove(path_ptr);
#endif
    DECREF(Hash_Delete(ivars->entries, (Obj*)name));
    FREEMEM(path_ptr);
    return result;
}

void
FSFolder_Close_IMP(FSFolder *self) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    Hash_Clear(ivars->entries);
}

Folder*
FSFolder_Local_Find_Folder_IMP(FSFolder *self, String *name) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);

    Folder *subfolder = NULL;
    if (!name || !Str_Get_Size(name)) {
        // No entity can be identified by NULL or empty string.
        return NULL;
    }
    else if (!S_is_local_entry(name)) {
        return NULL;
    }
    else if (Str_Starts_With_Utf8(name, ".", 1)) {
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
        String *cfmeta_file = (String*)SSTR_WRAP_UTF8("cfmeta.json", 11);
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
S_fullpath(FSFolder *self, String *path) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    String *fullpath = Str_newf("%o%s%o", ivars->path, CHY_DIR_SEP, path);
    String *retval;
    if (CHY_DIR_SEP[0] != '/') {
        retval = Str_Swap_Chars(fullpath, '/', CHY_DIR_SEP[0]);
        DECREF(fullpath);
    }
    else {
        retval = fullpath;
    }
    return retval;
}

static char*
S_fullpath_ptr(FSFolder *self, String *path) {
    FSFolderIVARS *const ivars = FSFolder_IVARS(self);
    size_t folder_size = Str_Get_Size(ivars->path);
    size_t path_size   = Str_Get_Size(path);
    size_t full_size   = folder_size + 1 + path_size;
    const char *folder_ptr = Str_Get_Ptr8(ivars->path);
    const char *path_ptr   = Str_Get_Ptr8(path);

    char *buf = (char*)MALLOCATE(full_size + 1);
    memcpy(buf, folder_ptr, folder_size);
    buf[folder_size] = CHY_DIR_SEP[0];
    memcpy(buf + folder_size + 1, path_ptr, path_size);
    buf[full_size] = '\0';

    if (CHY_DIR_SEP[0] != '/') {
        for (size_t i = 0; i < full_size; ++i) {
            if (buf[i] == '/') { buf[i] = CHY_DIR_SEP[0]; }
        }
    }

    return buf;
}

static bool
S_dir_ok(String *path) {
    bool retval = false;
    char *path_ptr = Str_To_Utf8(path);
    struct stat stat_buf;
    if (stat(path_ptr, &stat_buf) != -1) {
        if (stat_buf.st_mode & S_IFDIR) { retval = true; }
    }
    FREEMEM(path_ptr);
    return retval;
}

static bool
S_create_dir(String *path) {
    bool retval = true;
    char *path_ptr = Str_To_Utf8(path);
    if (-1 == chy_makedir(path_ptr, 0777)) {
        Err_set_error(Err_new(Str_newf("Couldn't create directory '%o': %s",
                                       path, strerror(errno))));
        retval = false;
    }
    FREEMEM(path_ptr);
    return retval;
}

static bool
S_is_local_entry(String *path) {
    return Str_Find_Utf8(path, "/", 1) == -1;
}

/***************************************************************************/

#if (defined(CHY_HAS_WINDOWS_H) && !defined(__CYGWIN__))

// Windows.h defines INCREF and DECREF, so we include it only at the end of
// this file and undef those symbols.
#undef INCREF
#undef DECREF

#include <windows.h>

static bool
S_is_absolute(String *path) {
    int32_t code_point = Str_Code_Point_At(path, 0);

    if (isalpha(code_point)) {
        code_point = Str_Code_Point_At(path, 1);
        if (code_point != ':') { return false; }
        code_point = Str_Code_Point_At(path, 2);
    }

    return code_point == '\\' || code_point == '/';
}

static String*
S_absolutify(String *path) {
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
S_hard_link(char *from8, char *to8) {
    if (CreateHardLink(to8, from8, NULL)) {
        return true;
    }
    else {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(Str_newf("CreateHardLink for new file '%s' from '%s' failed: %s",
                                       to8, from8, win_error)));
        FREEMEM(win_error);
        return false;
    }
}

#elif (defined(CHY_HAS_UNISTD_H))

static bool
S_is_absolute(String *path) {
    return Str_Starts_With_Utf8(path, CHY_DIR_SEP, 1);
}

static String*
S_absolutify(String *path) {
    if (S_is_absolute(path)) { return Str_Clone(path); }

    char *cwd = NULL;
    for (size_t buf_size = 256; buf_size <= 65536; buf_size *= 2) {
        cwd = (char*)MALLOCATE(buf_size);
        if (getcwd(cwd, buf_size)) { break; }
        FREEMEM(cwd);
        cwd = NULL;
        if (errno != ERANGE) { THROW(ERR, "getcwd failed"); }
    }
    if (!cwd) { THROW(ERR, "getcwd failed"); }
    String *abs_path = Str_newf("%s" CHY_DIR_SEP "%o", cwd, path);
    FREEMEM(cwd);

    return abs_path;
}

static bool
S_hard_link(char *from8, char *to8) {
    if (-1 == link(from8, to8)) {
        Err_set_error(Err_new(Str_newf("hard link for new file '%s' from '%s' failed: %s",
                                       to8, from8, strerror(errno))));
        return false;
    }
    else {
        return true;
    }
}

#else
  #error "Need either windows.h or unistd.h"
#endif /* CHY_HAS_UNISTD_H vs. CHY_HAS_WINDOWS_H */


