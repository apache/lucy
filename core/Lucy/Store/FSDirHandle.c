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

#define C_LUCY_FSDIRHANDLE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "Lucy/Util/ToolSet.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/VArray.h"
#include "Lucy/Store/FSDirHandle.h"

#ifdef CHY_HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif

FSDirHandle*
FSDH_open(const CharBuf *dir) {
    FSDirHandle *self = (FSDirHandle*)VTable_Make_Obj(FSDIRHANDLE);
    return FSDH_do_open(self, dir);
}

void
FSDH_destroy(FSDirHandle *self) {
    // Throw away saved error -- it's too late to call Close() now.
    DECREF(self->saved_error);
    self->saved_error = NULL;
    SUPER_DESTROY(self, FSDIRHANDLE);
}

static INLINE bool_t
SI_is_updir(const char *name, size_t len) {
    if (len == 2 && strncmp(name, "..", 2) == 0) {
        return true;
    }
    else if (len == 1 && name[0] ==  '.') {
        return true;
    }
    else {
        return false;
    }
}

/********************************** UNIXEN *********************************/
#if (defined(CHY_HAS_DIRENT_H) && !defined(CHY_HAS_WINDOWS_H))

#include <dirent.h>

FSDirHandle*
FSDH_do_open(FSDirHandle *self, const CharBuf *dir) {
    char *dir_path_ptr = (char*)CB_Get_Ptr8(dir);

    DH_init((DirHandle*)self, dir);
    self->sys_dir_entry    = NULL;
    self->fullpath         = NULL;

    self->sys_dirhandle = opendir(dir_path_ptr);
    if (!self->sys_dirhandle) {
        Err_set_error(Err_new(CB_newf("Failed to opendir '%o'", dir)));
        DECREF(self);
        return NULL;
    }

    return self;
}

bool_t
FSDH_next(FSDirHandle *self) {
    self->sys_dir_entry = (struct dirent*)readdir((DIR*)self->sys_dirhandle);
    if (!self->sys_dir_entry) {
        CB_Set_Size(self->entry, 0);
        return false;
    }
    else {
        struct dirent *sys_dir_entry = (struct dirent*)self->sys_dir_entry;
        #ifdef CHY_HAS_DIRENT_D_NAMLEN
        size_t len = sys_dir_entry->d_namlen;
        #else
        size_t len = strlen(sys_dir_entry->d_name);
        #endif
        if (SI_is_updir(sys_dir_entry->d_name, len)) {
            return FSDH_Next(self);
        }
        else {
            CB_Mimic_Str(self->entry, sys_dir_entry->d_name, len);
            return true;
        }
    }
}

bool_t
FSDH_entry_is_dir(FSDirHandle *self) {
    struct dirent *sys_dir_entry = (struct dirent*)self->sys_dir_entry;
    if (!sys_dir_entry) { return false; }

    // If d_type is available, try to avoid a stat() call.  If it's not, or if
    // the type comes back as unknown, fall back to stat().
    #ifdef CHY_HAS_DIRENT_D_TYPE
    if (sys_dir_entry->d_type == DT_DIR) {
        return true;
    }
    else if (sys_dir_entry->d_type != DT_UNKNOWN) {
        return false;
    }
    #endif

    struct stat stat_buf;
    if (!self->fullpath) {
        self->fullpath = CB_new(CB_Get_Size(self->dir) + 20);
    }
    CB_setf(self->fullpath, "%o%s%o", self->dir, CHY_DIR_SEP,
            self->entry);
    if (stat((char*)CB_Get_Ptr8(self->fullpath), &stat_buf) != -1) {
        if (stat_buf.st_mode & S_IFDIR) { return true; }
    }
    return false;
}

bool_t
FSDH_entry_is_symlink(FSDirHandle *self) {
    struct dirent *sys_dir_entry = (struct dirent*)self->sys_dir_entry;
    if (!sys_dir_entry) { return false; }

    #ifdef CHY_HAS_DIRENT_D_TYPE
    return sys_dir_entry->d_type == DT_LNK ? true : false;
    #else
    {
        struct stat stat_buf;
        if (!self->fullpath) {
            self->fullpath = CB_new(CB_Get_Size(self->dir) + 20);
        }
        CB_setf(self->fullpath, "%o%s%o", self->dir, CHY_DIR_SEP,
                self->entry);
        if (stat((char*)CB_Get_Ptr8(self->fullpath), &stat_buf) != -1) {
            if (stat_buf.st_mode & S_IFLNK) { return true; }
        }
        return false;
    }
    #endif // CHY_HAS_DIRENT_D_TYPE
}

bool_t
FSDH_close(FSDirHandle *self) {
    if (self->fullpath) {
        CB_Dec_RefCount(self->fullpath);
        self->fullpath = NULL;
    }
    if (self->sys_dirhandle) {
        DIR *sys_dirhandle = (DIR*)self->sys_dirhandle;
        self->sys_dirhandle = NULL;
        if (closedir(sys_dirhandle) == -1) {
            Err_set_error(Err_new(CB_newf("Error closing dirhandle: %s",
                                          strerror(errno))));
            return false;
        }
    }
    return true;
}

/********************************** Windows ********************************/
#elif defined(CHY_HAS_WINDOWS_H)

#include <windows.h>

FSDirHandle*
FSDH_do_open(FSDirHandle *self, const CharBuf *dir) {
    size_t  dir_path_size = CB_Get_Size(dir);
    char   *dir_path_ptr  = (char*)CB_Get_Ptr8(dir);
    char    search_string[MAX_PATH + 1];
    char   *path_ptr = search_string;

    DH_init((DirHandle*)self, dir);
    self->sys_dir_entry    = MALLOCATE(sizeof(WIN32_FIND_DATA));
    self->sys_dirhandle    = INVALID_HANDLE_VALUE;
    self->saved_error      = NULL;

    if (dir_path_size >= MAX_PATH - 2) {
        // Deal with Windows ceiling on file path lengths.
        Err_set_error(Err_new(CB_newf("Directory path is too long: %o",
                                      dir)));
        DECREF(self);
        return NULL;
    }

    // Append trailing wildcard so Windows lists dir contents rather than just
    // the dir name itself.
    memcpy(path_ptr, dir_path_ptr, dir_path_size);
    memcpy(path_ptr + dir_path_size, "\\*\0", 3);

    self->sys_dirhandle
        = FindFirstFile(search_string, (WIN32_FIND_DATA*)self->sys_dir_entry);
    if (INVALID_HANDLE_VALUE == self->sys_dirhandle) {
        // Directory inaccessible or doesn't exist.
        Err_set_error(Err_new(CB_newf("Failed to open dir '%o'", dir)));
        DECREF(self);
        return NULL;
    }
    else {
        // Compensate for the fact that FindFirstFile has already returned the
        // first entry but DirHandle's API requires that you call Next() to
        // start the iterator.
        self->delayed_iter = true;
    }

    return self;
}

bool_t
FSDH_entry_is_dir(FSDirHandle *self) {
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)self->sys_dir_entry;
    if (find_data) {
        if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            return true;
        }
    }
    return false;
}

bool_t
FSDH_entry_is_symlink(FSDirHandle *self) {
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)self->sys_dir_entry;
    if (find_data) {
        if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            return true;
        }
    }
    return false;
}

bool_t
FSDH_close(FSDirHandle *self) {
    if (self->sys_dirhandle && self->sys_dirhandle != INVALID_HANDLE_VALUE) {
        HANDLE dirhandle = (HANDLE)self->sys_dirhandle;
        self->sys_dirhandle = NULL;
        if (dirhandle != INVALID_HANDLE_VALUE && !FindClose(dirhandle)) {
            if (!self->saved_error) {
                char *win_error = Err_win_error();
                self->saved_error
                    = Err_new(CB_newf("Error while closing directory: %s",
                                      win_error));
                FREEMEM(win_error);
            }
        }
    }
    if (self->sys_dir_entry) {
        FREEMEM(self->sys_dir_entry);
        self->sys_dir_entry = NULL;
    }

    // If we encountered an error condition previously, report it now.
    if (self->saved_error) {
        Err_set_error((Err*)INCREF(self->saved_error));
        return false;
    }
    else {
        return true;
    }
}

bool_t
FSDH_next(FSDirHandle *self) {
    HANDLE           dirhandle = (HANDLE)self->sys_dirhandle;
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)self->sys_dir_entry;

    // Attempt to move forward or absorb cached iter.
    if (!dirhandle || dirhandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    else if (self->delayed_iter) {
        self->delayed_iter = false;
    }
    else if ((FindNextFile(dirhandle, find_data) == 0)) {
        // Iterator exhausted.  Verify that no errors were encountered.
        CB_Set_Size(self->entry, 0);
        if (GetLastError() != ERROR_NO_MORE_FILES) {
            char *win_error = Err_win_error();
            self->saved_error
                = Err_new(CB_newf("Error while traversing directory: %s",
                                  win_error));
            FREEMEM(win_error);
        }
        return false;
    }

    // Process the results of the iteration.
    {
        size_t len = strlen(find_data->cFileName);
        if (SI_is_updir(find_data->cFileName, len)) {
            return FSDH_Next(self);
        }
        else {
            CB_Mimic_Str(self->entry, find_data->cFileName, len);
            return true;
        }
    }
}

#else
  #error "Need either dirent.h or windows.h"
#endif // CHY_HAS_DIRENT_H vs. CHY_HAS_WINDOWS_H


