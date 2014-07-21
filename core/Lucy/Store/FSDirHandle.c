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

#include "charmony.h"

#include "Lucy/Util/ToolSet.h"
#include "Clownfish/Err.h"
#include "Clownfish/VArray.h"
#include "Lucy/Store/FSDirHandle.h"

#ifdef CHY_HAS_SYS_TYPES_H
  #include <sys/types.h>
#endif

FSDirHandle*
FSDH_open(String *dir) {
    FSDirHandle *self = (FSDirHandle*)Class_Make_Obj(FSDIRHANDLE);
    return FSDH_do_open(self, dir);
}

void
FSDH_Destroy_IMP(FSDirHandle *self) {
    // Throw away saved error -- it's too late to call Close() now.
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    DECREF(ivars->saved_error);
    ivars->saved_error = NULL;
    SUPER_DESTROY(self, FSDIRHANDLE);
}

static CFISH_INLINE bool
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

/********************************** Windows ********************************/
#if (defined(CHY_HAS_WINDOWS_H) && !defined(__CYGWIN__))

#include <windows.h>

FSDirHandle*
FSDH_do_open(FSDirHandle *self, String *dir) {
    size_t      dir_path_size = Str_Get_Size(dir);
    const char *dir_path_ptr  = Str_Get_Ptr8(dir);
    char        search_string[MAX_PATH + 1];
    char       *path_ptr = search_string;

    DH_init((DirHandle*)self, dir);
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    ivars->sys_dir_entry    = MALLOCATE(sizeof(WIN32_FIND_DATA));
    ivars->sys_dirhandle    = INVALID_HANDLE_VALUE;
    ivars->saved_error      = NULL;

    if (dir_path_size >= MAX_PATH - 2) {
        // Deal with Windows ceiling on file path lengths.
        Err_set_error(Err_new(Str_newf("Directory path is too long: %o",
                                       dir)));
        CFISH_DECREF(self);
        return NULL;
    }

    // Append trailing wildcard so Windows lists dir contents rather than just
    // the dir name itself.
    memcpy(path_ptr, dir_path_ptr, dir_path_size);
    memcpy(path_ptr + dir_path_size, "\\*\0", 3);

    ivars->sys_dirhandle
        = FindFirstFile(search_string, (WIN32_FIND_DATA*)ivars->sys_dir_entry);
    if (INVALID_HANDLE_VALUE == ivars->sys_dirhandle) {
        // Directory inaccessible or doesn't exist.
        Err_set_error(Err_new(Str_newf("Failed to open dir '%o'", dir)));
        CFISH_DECREF(self);
        return NULL;
    }
    else {
        // Compensate for the fact that FindFirstFile has already returned the
        // first entry but DirHandle's API requires that you call Next() to
        // start the iterator.
        ivars->delayed_iter = true;
    }

    return self;
}

bool
FSDH_Entry_Is_Dir_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)ivars->sys_dir_entry;
    if (find_data) {
        if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            return true;
        }
    }
    return false;
}

bool
FSDH_Entry_Is_Symlink_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)ivars->sys_dir_entry;
    if (find_data) {
        if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
            return true;
        }
    }
    return false;
}

bool
FSDH_Close_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    if (ivars->sys_dirhandle && ivars->sys_dirhandle != INVALID_HANDLE_VALUE) {
        HANDLE dirhandle = (HANDLE)ivars->sys_dirhandle;
        ivars->sys_dirhandle = NULL;
        if (dirhandle != INVALID_HANDLE_VALUE && !FindClose(dirhandle)) {
            if (!ivars->saved_error) {
                char *win_error = Err_win_error();
                ivars->saved_error
                    = Err_new(Str_newf("Error while closing directory: %s",
                                       win_error));
                FREEMEM(win_error);
            }
        }
    }
    if (ivars->sys_dir_entry) {
        FREEMEM(ivars->sys_dir_entry);
        ivars->sys_dir_entry = NULL;
    }

    // If we encountered an error condition previously, report it now.
    if (ivars->saved_error) {
        Err_set_error((Err*)CFISH_INCREF(ivars->saved_error));
        return false;
    }
    else {
        return true;
    }
}

bool
FSDH_Next_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    HANDLE           dirhandle = (HANDLE)ivars->sys_dirhandle;
    WIN32_FIND_DATA *find_data = (WIN32_FIND_DATA*)ivars->sys_dir_entry;

    // Attempt to move forward or absorb cached iter.
    if (!dirhandle || dirhandle == INVALID_HANDLE_VALUE) {
        return false;
    }
    else if (ivars->delayed_iter) {
        ivars->delayed_iter = false;
    }
    else if ((FindNextFile(dirhandle, find_data) == 0)) {
        // Iterator exhausted.  Verify that no errors were encountered.
        CFISH_DECREF(ivars->entry);
        ivars->entry = NULL;
        if (GetLastError() != ERROR_NO_MORE_FILES) {
            char *win_error = Err_win_error();
            ivars->saved_error
                = Err_new(Str_newf("Error while traversing directory: %s",
                                   win_error));
            FREEMEM(win_error);
        }
        return false;
    }

    // Process the results of the iteration.
    size_t len = strlen(find_data->cFileName);
    if (SI_is_updir(find_data->cFileName, len)) {
        return FSDH_Next(self);
    }
    else {
        CFISH_DECREF(ivars->entry);
        ivars->entry = Str_new_from_utf8(find_data->cFileName, len);
        return true;
    }
}

/********************************** UNIXEN *********************************/
#elif defined(CHY_HAS_DIRENT_H)

#include <dirent.h>

FSDirHandle*
FSDH_do_open(FSDirHandle *self, String *dir) {
    DH_init((DirHandle*)self, dir);
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    ivars->sys_dir_entry = NULL;

    char *dir_path_ptr = Str_To_Utf8(dir);
    ivars->sys_dirhandle = opendir(dir_path_ptr);
    FREEMEM(dir_path_ptr);
    if (!ivars->sys_dirhandle) {
        Err_set_error(Err_new(Str_newf("Failed to opendir '%o'", dir)));
        DECREF(self);
        return NULL;
    }

    return self;
}

bool
FSDH_Next_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    ivars->sys_dir_entry = (struct dirent*)readdir((DIR*)ivars->sys_dirhandle);
    if (!ivars->sys_dir_entry) {
        DECREF(ivars->entry);
        ivars->entry = NULL;
        return false;
    }
    else {
        struct dirent *sys_dir_entry = (struct dirent*)ivars->sys_dir_entry;
        #ifdef CHY_HAS_DIRENT_D_NAMLEN
        size_t len = sys_dir_entry->d_namlen;
        #else
        size_t len = strlen(sys_dir_entry->d_name);
        #endif
        if (SI_is_updir(sys_dir_entry->d_name, len)) {
            return FSDH_Next(self);
        }
        else {
            DECREF(ivars->entry);
            ivars->entry = Str_new_from_utf8(sys_dir_entry->d_name, len);
            return true;
        }
    }
}

bool
FSDH_Entry_Is_Dir_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    struct dirent *sys_dir_entry = (struct dirent*)ivars->sys_dir_entry;
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

    bool retval = false;
    struct stat stat_buf;
    String *fullpath = Str_newf("%o%s%o", ivars->dir, CHY_DIR_SEP,
                                ivars->entry);
    char *fullpath_ptr = Str_To_Utf8(fullpath);
    if (stat(fullpath_ptr, &stat_buf) != -1) {
        if (stat_buf.st_mode & S_IFDIR) { retval = true; }
    }
    FREEMEM(fullpath_ptr);
    DECREF(fullpath);
    return retval;
}

bool
FSDH_Entry_Is_Symlink_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    struct dirent *sys_dir_entry = (struct dirent*)ivars->sys_dir_entry;
    if (!sys_dir_entry) { return false; }

    #ifdef CHY_HAS_DIRENT_D_TYPE
    return sys_dir_entry->d_type == DT_LNK ? true : false;
    #else
    {
        bool retval = false;
        struct stat stat_buf;
        String *fullpath = Str_newf("%o%s%o", ivars->dir, CHY_DIR_SEP,
                                    ivars->entry);
        char *fullpath_ptr = Str_To_Utf8(fullpath);
        if (stat(fullpath_ptr, &stat_buf) != -1) {
            if (stat_buf.st_mode & S_IFLNK) { retval = true; }
        }
        FREEMEM(fullpath_ptr);
        DECREF(fullpath);
        return retval;
    }
    #endif // CHY_HAS_DIRENT_D_TYPE
}

bool
FSDH_Close_IMP(FSDirHandle *self) {
    FSDirHandleIVARS *const ivars = FSDH_IVARS(self);
    if (ivars->sys_dirhandle) {
        DIR *sys_dirhandle = (DIR*)ivars->sys_dirhandle;
        ivars->sys_dirhandle = NULL;
        if (closedir(sys_dirhandle) == -1) {
            Err_set_error(Err_new(Str_newf("Error closing dirhandle: %s",
                                           strerror(errno))));
            return false;
        }
    }
    return true;
}

#else
  #error "Need either dirent.h or windows.h"
#endif // CHY_HAS_DIRENT_H vs. CHY_HAS_WINDOWS_H


