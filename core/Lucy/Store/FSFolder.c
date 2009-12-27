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

/* For rmdir, (hard) link. */
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

/* For CreateHardLink. */
#ifdef CHY_HAS_WINDOWS_H
  #include <windows.h>
#endif

/* For mkdir, rmdir. */
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

/* Return a new CharBuf containing a platform-specific absolute filepath. */
static CharBuf*
S_fullpath(FSFolder *self, const CharBuf *path);

/* Return true if the supplied path is a directory. */
static bool_t
S_dir_ok(const CharBuf *path);

/* Create a directory, or set Err_error and return false. */
bool_t
S_create_dir(const CharBuf *path);

/* Return true unless the supplied path contains a slash. */
bool_t
S_is_local_entry(const CharBuf *path);

FSFolder*
FSFolder_new(const CharBuf *path) 
{
    FSFolder *self = (FSFolder*)VTable_Make_Obj(FSFOLDER);
    return FSFolder_init(self, path);
}

FSFolder*
FSFolder_init(FSFolder *self, const CharBuf *path)
{
    CharBuf *abs_path = FSFolder_absolutify(path);
    Folder_init((Folder*)self, abs_path);
    DECREF(abs_path);
    return self;
}

void
FSFolder_initialize(FSFolder *self)
{
    if (!S_dir_ok(self->path)) {
        if (!S_create_dir(self->path)) {
            RETHROW(INCREF(Err_get_error()));
        }
    }
}

bool_t
FSFolder_check(FSFolder *self)
{
    return S_dir_ok(self->path);
}

FileHandle*
FSFolder_local_open_filehandle(FSFolder *self, const CharBuf *name, u32_t flags)
{
    CharBuf      *fullpath = S_fullpath(self, name);
    FSFileHandle *fh = FSFH_open(fullpath, flags);
    if (!fh) {
        ERR_ADD_FRAME(Err_get_error());
    }
    DECREF(fullpath);
    return (FileHandle*)fh;
}

bool_t
FSFolder_local_mkdir(FSFolder *self, const CharBuf *name)
{
    CharBuf *dir = S_fullpath(self, name);
    bool_t result = S_create_dir(dir);
    DECREF(dir);
    if (!result) {
        ERR_ADD_FRAME(Err_get_error());
    }
    return result;
}

DirHandle*
FSFolder_local_open_dir(FSFolder *self)
{
    DirHandle *dh = (DirHandle*)FSDH_open(self->path);
    if (!dh) { ERR_ADD_FRAME(Err_get_error()); }
    return dh;
}

bool_t
FSFolder_local_exists(FSFolder *self, const CharBuf *name)
{
    if (Hash_Fetch(self->entries, (Obj*)name)) {
        return true;
    }
    else if (!S_is_local_entry(name)) {
        return false;
    }
    else {
        struct stat stat_buf;
        CharBuf *fullpath = S_fullpath(self, name);
        bool_t retval = false;
        if (stat((char*)CB_Get_Ptr8(fullpath), &stat_buf) != -1)
            retval = true;
        DECREF(fullpath);
        return retval;
    }
}

bool_t
FSFolder_local_is_directory(FSFolder *self, const CharBuf *name)
{
    /* Check for a cached object, then fall back to a system call. */
    Obj *elem = Hash_Fetch(self->entries, (Obj*)name);
    if (elem && Obj_Is_A(elem, FOLDER)) { 
        return true; 
    }
    else {
        CharBuf *fullpath = S_fullpath(self, name);
        bool_t result = S_dir_ok(fullpath);
        DECREF(fullpath);
        return result;
    }
}

bool_t
FSFolder_rename(FSFolder *self, const CharBuf* from, const CharBuf *to)
{
    CharBuf *from_path = S_fullpath(self, from);
    CharBuf *to_path   = S_fullpath(self, to);
    bool_t   retval    = !rename((char*)CB_Get_Ptr8(from_path), 
        (char*)CB_Get_Ptr8(to_path));
    if (!retval) {
        Err_set_error(Err_new(CB_newf("rename from '%o' to '%o' failed: %s",
            from_path, to_path, strerror(errno))));
    }
    DECREF(to_path);
    DECREF(from_path);
    return retval;
}

bool_t
FSFolder_hard_link(FSFolder *self, const CharBuf *from, 
                   const CharBuf *to)
{
    CharBuf *from_path = S_fullpath(self, from);
    CharBuf *to_path   = S_fullpath(self, to);
    char    *from8     = (char*)CB_Get_Ptr8(from_path);
    char    *to8       = (char*)CB_Get_Ptr8(to_path);
#ifdef CHY_HAS_UNISTD_H
    bool_t retval;
    if (-1 == link(from8, to8)) {
        retval = false;
        Err_set_error(Err_new(CB_newf(
            "hard link for new file '%o' from '%o' failed: %s",
                to_path, from_path, strerror(errno))));
    }
    else {
        retval = true;
    }
#elif defined(CHY_HAS_WINDOWS_H)
    bool_t retval = !!CreateHardLink(to8, from8, NULL);
    if (!retval) {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(CB_newf(
            "CreateHardLink for new file '%o' from '%o' failed: %s",
                to_path, from_path, win_error)));
        FREEMEM(win_error);
    }
#endif
    DECREF(to_path);
    DECREF(from_path);
    return retval;
}

bool_t
FSFolder_local_delete(FSFolder *self, const CharBuf *name)
{
    CharBuf *fullpath = S_fullpath(self, name);
    char    *path_ptr = (char*)CB_Get_Ptr8(fullpath);
#ifdef CHY_REMOVE_ZAPS_DIRS
    bool_t result = !remove(path_ptr);
#else 
    bool_t result = !rmdir(path_ptr) || !remove(path_ptr); 
#endif
    DECREF(fullpath);
    DECREF(Hash_Delete(self->entries, (Obj*)name));
    return result;
}

void
FSFolder_close(FSFolder *self)
{
    Hash_Clear(self->entries);
}

Folder*
FSFolder_local_find_folder(FSFolder *self, const CharBuf *name)
{
    Folder *subfolder = NULL;
    if (!name || !CB_Get_Size(name)) {
        /* No entity can be identified by NULL or empty string. */
        return NULL;
    }
    else if (!S_is_local_entry(name)) {
        return NULL;
    }
    else if (NULL != (subfolder = (Folder*)Hash_Fetch(self->entries, (Obj*)name))) {
        if (Folder_Is_A(subfolder, FOLDER)) {
            return subfolder;
        }
        else {
            return NULL;
        }
    }

    {
        CharBuf *fullpath = S_fullpath(self, name);
        static ZombieCharBuf cfmeta_file = ZCB_LITERAL("cfmeta.json");
        if (S_dir_ok(fullpath)) {
            subfolder = (Folder*)FSFolder_new(fullpath);
            if (!subfolder) {
                THROW(ERR, "Failed to open FSFolder at '%o'", fullpath);
            }
            /* Try to open a CompoundFileReader. On failure, just use the
             * existing folder. */
            if (Folder_Local_Exists(subfolder, (CharBuf*)&cfmeta_file)) {
                CompoundFileReader *cf_reader = CFReader_open(subfolder);
                if (cf_reader) {
                    DECREF(subfolder);
                    subfolder = (Folder*)cf_reader;
                }
            }
            Hash_Store(self->entries, (Obj*)name, (Obj*)subfolder);
            DECREF(fullpath);
            return subfolder;
        }
        else {
            DECREF(fullpath);
        }
    }

    return NULL;
}

static CharBuf*
S_fullpath(FSFolder *self, const CharBuf *path)
{
    CharBuf *fullpath = CB_new(200);
    CB_Cat(fullpath, self->path);
    CB_Cat_Trusted_Str(fullpath, DIR_SEP, sizeof(DIR_SEP) - 1);
    CB_Cat(fullpath, path);
    if (DIR_SEP[0] != '/') {
        CB_Swap_Chars(fullpath, '/', DIR_SEP[0]);
    }
    return fullpath;
}

static bool_t
S_dir_ok(const CharBuf *path)
{
    struct stat stat_buf;
    if (stat((char*)CB_Get_Ptr8(path), &stat_buf) != -1) {
        if (stat_buf.st_mode & S_IFDIR) return true;
    }
    return false;
}

bool_t
S_create_dir(const CharBuf *path)
{
    if (-1 == chy_makedir((char*)CB_Get_Ptr8(path), 0777)) {
        Err_set_error(Err_new(CB_newf(
            "Couldn't create directory '%o': %s", path, strerror(errno))));
        return false;
    }
    return true;
}

bool_t
S_is_local_entry(const CharBuf *path)
{
    ZombieCharBuf scratch = ZCB_make(path);
    u32_t code_point;
    while (0 != (code_point = ZCB_Nip_One(&scratch))) {
        if (code_point == '/') { return false; }
    }
    return true;
}

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

