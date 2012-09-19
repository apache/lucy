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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#include "CFCUtil.h"

void
CFCUtil_null_check(const void *arg, const char *name, const char *file,
                   int line) {
    if (!arg) {
        CFCUtil_die("%s cannot be NULL at %s line %d", name, file, line);
    }
}

char*
CFCUtil_strdup(const char *string) {
    if (!string) { return NULL; }
    return CFCUtil_strndup(string, strlen(string));
}

char*
CFCUtil_strndup(const char *string, size_t len) {
    if (!string) { return NULL; }
    char *copy = (char*)MALLOCATE(len + 1);
    memcpy(copy, string, len);
    copy[len] = '\0';
    return copy;
}

char*
CFCUtil_cat(char *string, ...) {
    va_list args;
    char *appended;
    CFCUTIL_NULL_CHECK(string);
    size_t size = strlen(string) + 1;
    va_start(args, string);
    while (NULL != (appended = va_arg(args, char*))) {
        size += strlen(appended);
        string = (char*)REALLOCATE(string, size);
        strcat(string, appended);
    }
    va_end(args);
    return string;
}

void
CFCUtil_trim_whitespace(char *text) {
    if (!text) {
        return;
    }

    // Find start.
    char *ptr = text;
    while (*ptr != '\0' && isspace(*ptr)) { ptr++; }

    // Find end.
    size_t orig_len = strlen(text);
    char *limit = text + orig_len;
    for (; limit > text; limit--) {
        if (!isspace(*(limit - 1))) { break; }
    }

    // Modify string in place and NULL-terminate.
    while (ptr < limit) {
        *text++ = *ptr++;
    }
    *text = '\0';
}

void*
CFCUtil_wrapped_malloc(size_t count, const char *file, int line) {
    void *pointer = malloc(count);
    if (pointer == NULL && count != 0) {
        if (sizeof(long) >= sizeof(size_t)) {
            fprintf(stderr, "Can't malloc %lu bytes at %s line %d\n",
                    (unsigned long)count, file, line);
        }
        else {
            fprintf(stderr, "malloc failed at %s line %d\n", file, line);
        }
        exit(1);
    }
    return pointer;
}

void*
CFCUtil_wrapped_calloc(size_t count, size_t size, const char *file, int line) {
    void *pointer = calloc(count, size);
    if (pointer == NULL && count != 0) {
        if (sizeof(long) >= sizeof(size_t)) {
            fprintf(stderr,
                    "Can't calloc %lu elements of size %lu at %s line %d\n",
                    (unsigned long)count, (unsigned long)size, file, line);
        }
        else {
            fprintf(stderr, "calloc failed at %s line %d\n", file, line);
        }
        exit(1);
    }
    return pointer;
}

void*
CFCUtil_wrapped_realloc(void *ptr, size_t size, const char *file, int line) {
    void *pointer = realloc(ptr, size);
    if (pointer == NULL && size != 0) {
        if (sizeof(long) >= sizeof(size_t)) {
            fprintf(stderr, "Can't realloc %lu bytes at %s line %d\n",
                    (unsigned long)size, file, line);
        }
        else {
            fprintf(stderr, "realloc failed at %s line %d\n", file, line);
        }
        exit(1);
    }
    return pointer;
}

void
CFCUtil_wrapped_free(void *ptr) {
    free(ptr);
}

int
CFCUtil_current(const char *orig, const char *dest) {
    // If the destination file doesn't exist, we're not current.
    struct stat dest_stat;
    if (stat(dest, &dest_stat) == -1) {
        return false;
    }

    // If the source file is newer than the dest, we're not current.
    struct stat orig_stat;
    if (stat(orig, &orig_stat) == -1) {
        CFCUtil_die("Missing source file '%s'", orig);
    }
    if (orig_stat.st_mtime > dest_stat.st_mtime) {
        return false;
    }

    // Current!
    return 1;
}

void
CFCUtil_write_file(const char *filename, const char *content, size_t len) {
    FILE *fh = fopen(filename, "w+");
    if (fh == NULL) {
        CFCUtil_die("Couldn't open '%s': %s", filename, strerror(errno));
    }
    fwrite(content, sizeof(char), len, fh);
    if (fclose(fh)) {
        CFCUtil_die("Error when closing '%s': %s", filename, strerror(errno));
    }
}

char*
CFCUtil_slurp_text(const char *file_path, size_t *len_ptr) {
    FILE   *const file = fopen(file_path, "r");
    char   *contents;
    size_t  binary_len;
    long    text_len;

    /* Sanity check. */
    if (file == NULL) {
        CFCUtil_die("Error opening file '%s': %s", file_path, strerror(errno));
    }

    /* Find length; return NULL if the file has a zero-length. */
    binary_len = CFCUtil_flength(file);
    if (binary_len == 0) {
        *len_ptr = 0;
        return NULL;
    }

    /* Allocate memory and read the file. */
    contents = (char*)MALLOCATE(binary_len * sizeof(char) + 1);
    text_len = fread(contents, sizeof(char), binary_len, file);

    /* Weak error check, because CRLF might result in fewer chars read. */
    if (text_len <= 0) {
        CFCUtil_die("Tried to read %ld bytes of '%s', got return code %ld",
                    (long)binary_len, file_path, (long)text_len);
    }

    /* NULL-terminate. */
    contents[text_len] = '\0';

    /* Set length pointer for benefit of caller. */
    *len_ptr = text_len;

    /* Clean up. */
    if (fclose(file)) {
        CFCUtil_die("Error closing file '%s': %s", file_path, strerror(errno));
    }

    return contents;
}

int
CFCUtil_write_if_changed(const char *path, const char *content, size_t len) {
    FILE *f = fopen(path, "r");
    if (f) { // Does file exist?
        if (fclose(f)) {
            CFCUtil_die("Error closing file '%s': %s", path, strerror(errno));
        }
        size_t existing_len;
        char *existing = CFCUtil_slurp_text(path, &existing_len);
        int changed = true;
        if (existing_len == len && strcmp(content, existing) == 0) {
            changed = false;
        }
        FREEMEM(existing);
        if (changed == false) {
            return false;
        }
    }
    CFCUtil_write_file(path, content, len);
    return true;
}

long
CFCUtil_flength(void *file) {
    FILE *f = (FILE*)file;
    const long bookmark = (long)ftell(f);
    long check_val;
    long len;

    /* Seek to end of file and check length. */
    check_val = fseek(f, 0, SEEK_END);
    if (check_val == -1) { CFCUtil_die("fseek error : %s\n", strerror(errno)); }
    len = (long)ftell(f);
    if (len == -1) { CFCUtil_die("ftell error : %s\n", strerror(errno)); }

    /* Return to where we were. */
    check_val = fseek(f, bookmark, SEEK_SET);
    if (check_val == -1) { CFCUtil_die("fseek error : %s\n", strerror(errno)); }

    return len;
}

// Note: this has to be defined before including the Perl headers because they
// redefine stat() in an incompatible way on certain systems (Windows).
int
CFCUtil_is_dir(const char *path) {
    struct stat stat_buf;
    int stat_check = stat(path, &stat_buf);
    if (stat_check == -1) {
        return false;
    }
    return (stat_buf.st_mode & S_IFDIR) ? true : false;
}

int
CFCUtil_make_path(const char *path) {
    CFCUTIL_NULL_CHECK(path);
    char *target = CFCUtil_strdup(path);
    size_t orig_len = strlen(target);
    size_t len = orig_len;
    for (size_t i = 0; i <= len; i++) {
#ifndef WIN32
        if (target[i] == '\\') {
            i++;
            continue;
        }
#endif
        if (target[i] == CFCUTIL_PATH_SEP_CHAR || i == len) {
            target[i] = 0; // NULL-terminate.
            struct stat stat_buf;
            int stat_check = stat(target, &stat_buf);
            if (stat_check != -1) {
                if (!(stat_buf.st_mode & S_IFDIR)) {
                    CFCUtil_die("%s isn't a directory", target);
                }
            }
            else {
                int success = CFCUtil_make_dir(target);
                if (!success) {
                    FREEMEM(target);
                    return false;
                }
            }
            target[i] = CFCUTIL_PATH_SEP_CHAR;
        }
    }

    FREEMEM(target);
    return true;
}

void
CFCUtil_walk(const char *path, CFCUtil_walk_callback_t callback,
             void *context) {
    // If it's a valid file system entry, invoke the callback.
    struct stat stat_buf;
    int stat_check = stat(path, &stat_buf);
    if (stat_check == -1) {
        return;
    }
    callback(path, context);

    // Recurse into directories.
    if (!(stat_buf.st_mode & S_IFDIR)) {
        return;
    }
    void   *dirhandle   = CFCUtil_opendir(path);
    size_t  dir_len     = strlen(path);
    size_t  subpath_cap = dir_len * 2;
    char   *subpath     = (char*)MALLOCATE(subpath_cap);
    const char *entry   = NULL;
    while (NULL != (entry = CFCUtil_dirnext(dirhandle))) {
        if (strcmp(entry, ".") == 0 || strcmp(entry, "..") == 0) {
            continue;
        }
        size_t name_len = strlen(entry);
        size_t needed = dir_len + 1 + name_len + 1;
        if (needed > subpath_cap) {
            subpath_cap = needed;
            subpath = (char*)REALLOCATE(subpath, subpath_cap);
        }
        sprintf(subpath, "%s" CFCUTIL_PATH_SEP "%s", path, entry);
        CFCUtil_walk(subpath, callback, context);
    }
    FREEMEM(subpath);
    CFCUtil_closedir(dirhandle, path);
}

/******************************** WINDOWS **********************************/
#ifdef _WIN32

#include <direct.h>
#include <windows.h>

typedef struct WinDH {
    HANDLE handle;
    WIN32_FIND_DATA *find_data;
    char path[MAX_PATH + 1];
    int first_time;
} WinDH;

int
CFCUtil_make_dir(const char *dir) {
    return !mkdir(dir);
}

void*
CFCUtil_opendir(const char *dir) {
    size_t dirlen = strlen(dir);
    if (dirlen >= MAX_PATH - 2) {
        CFCUtil_die("Exceeded MAX_PATH(%d): %s", (int)MAX_PATH, dir);
    }
    WinDH *dh = (WinDH*)CALLOCATE(1, sizeof(WinDH));
    dh->find_data = (WIN32_FIND_DATA*)MALLOCATE(sizeof(WIN32_FIND_DATA));

    // Tack on wildcard needed by FindFirstFile.
    sprintf(dh->path, "%s\\*", dir);

    dh->handle = FindFirstFile(dh->path, dh->find_data);
    if (dh->handle == INVALID_HANDLE_VALUE) {
        CFCUtil_die("Can't open dir '%s'", dh->path);
    }
    dh->first_time = true;

    return dh;
}

const char*
CFCUtil_dirnext(void *dirhandle) {
    WinDH *dh = (WinDH*)dirhandle;
    if (dh->first_time) {
        dh->first_time = false;
    }
    else {
        if ((FindNextFile(dh->handle, dh->find_data) == 0)) {
            if (GetLastError() != ERROR_NO_MORE_FILES) {
                CFCUtil_die("Error occurred while reading '%s'",
                            dh->path);
            }
            return NULL;
        }
    }
    return dh->find_data->cFileName;
}

void
CFCUtil_closedir(void *dirhandle, const char *dir) {
    WinDH *dh = (WinDH*)dirhandle;
    if (!FindClose(dh->handle)) {
        CFCUtil_die("Error occurred while closing dir '%s'", dir);
    }
    FREEMEM(dh->find_data);
    FREEMEM(dh);
}

/******************************** UNIXEN ***********************************/
#else

#include <dirent.h>

int
CFCUtil_make_dir(const char *dir) {
    return !mkdir(dir, 0777);
}


void*
CFCUtil_opendir(const char *dir) {
    DIR *dirhandle = opendir(dir);
    if (!dirhandle) {
        CFCUtil_die("Failed to opendir for '%s': %s", dir, strerror(errno));
    }
    return dirhandle;
}

const char*
CFCUtil_dirnext(void *dirhandle) {
    struct dirent *entry = readdir((DIR*)dirhandle);
    return entry ? entry->d_name : NULL;
}

void
CFCUtil_closedir(void *dirhandle, const char *dir) {
    if (closedir((DIR*)dirhandle) == -1) {
        CFCUtil_die("Error closing dir '%s': %s", dir, strerror(errno));
    }
}

#endif /* Windows vs. Unix. */

/***************************************************************************/

#ifdef CFCPERL

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

void
CFCUtil_die(const char* format, ...) {
    SV *errsv = get_sv("@", 1);
    va_list args;
    va_start(args, format);
    sv_vsetpvf_mg(errsv, format, &args);
    va_end(args);
    croak(NULL);
}

void
CFCUtil_warn(const char* format, ...) {
    SV *mess = newSVpv("", 0);
    va_list args;
    va_start(args, format);
    sv_vsetpvf(mess, format, &args);
    va_end(args);
    fprintf(stderr, "%s\n", SvPV_nolen(mess));
    SvREFCNT_dec(mess);
}

#else

void
CFCUtil_die(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void
CFCUtil_warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#endif /* CFCPERL */

