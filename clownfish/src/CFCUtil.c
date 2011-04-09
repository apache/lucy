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
CFCUtil_null_check(const void *arg, const char *name, const char *file, int line)
{
    if (!arg) {
        CFCUtil_die("%s cannot be NULL at %s line %d", name, file, line);
    }
}

char*
CFCUtil_strdup(const char *string)
{
    if (!string) { return NULL; }
    return CFCUtil_strndup(string, strlen(string));
}

char*
CFCUtil_strndup(const char *string, size_t len)
{
    if (!string) { return NULL; }
    char *copy = (char*)MALLOCATE(len + 1);
    memcpy(copy, string, len);
    copy[len] = '\0';
    return copy;
}

void
CFCUtil_trim_whitespace(char *text)
{
    if (!text) {
        return;
    }

    // Find start.
    char *ptr = text;
    while (*ptr != '\0' && isspace(*ptr)) { ptr++; }

    // Find end.
    size_t orig_len = strlen(text);
    char *limit = text + orig_len;
    for ( ; limit > text; limit--) {
        if (!isspace(*(limit - 1))) { break; }
    }

    // Modify string in place and NULL-terminate.
    while (ptr < limit) {
        *text++ = *ptr++;
    }
    *text = '\0';
}

void*
CFCUtil_wrapped_malloc(size_t count, const char *file, int line)
{
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
CFCUtil_wrapped_calloc(size_t count, size_t size, const char *file, int line)
{
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
CFCUtil_wrapped_realloc(void *ptr, size_t size, const char *file, int line)
{
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
CFCUtil_wrapped_free(void *ptr)
{
    free(ptr);
}

int
CFCUtil_current(const char *orig, const char *dest)
{
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
CFCUtil_write_file(const char *filename, const char *content, size_t len)
{
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
CFCUtil_slurp_file(const char *file_path, size_t *len_ptr) 
{
    FILE   *const file = fopen(file_path, "rb");
    char   *contents;
    size_t  len;
    long    check_val;

    /* Sanity check. */
    if (file == NULL) {
        CFCUtil_die("Error opening file '%s': %s", file_path, strerror(errno));
    }

    /* Find length; return NULL if the file has a zero-length. */
    len = CFCUtil_flength(file);
    if (len == 0) {
        *len_ptr = 0;
        return NULL;
    }

    /* Allocate memory and read the file. */
    contents = (char*)MALLOCATE(len * sizeof(char) + 1);
    contents[len] = '\0';
    check_val = fread(contents, sizeof(char), len, file);

    /* Weak error check, because CRLF might result in fewer chars read. */
    if (check_val <= 0) {
        CFCUtil_die("Tried to read %d characters of '%s', got %d", (int)len,
            file_path, check_val);
    }

    /* Set length pointer for benefit of caller. */
    *len_ptr = check_val;

    /* Clean up. */
    if (fclose(file)) {
        CFCUtil_die("Error closing file '%s': %s", file_path, strerror(errno));
    }

    return contents;
}

void
CFCUtil_write_if_changed(const char *path, const char *content, size_t len)
{
    FILE *f = fopen(path, "r");
    if (f) { // Does file exist?
        if (fclose(f)) { 
            CFCUtil_die("Error closing file '%s': %s", path, strerror(errno));
        }
        size_t existing_len;
        char *existing = CFCUtil_slurp_file(path, &existing_len);
        int changed = true;
        if (existing_len == len && strcmp(content, existing) == 0) {
            changed = false;
        }
        FREEMEM(existing);
        if (changed == false) {
            return;
        }
    }
    CFCUtil_write_file(path, content, len);
}

long 
CFCUtil_flength(void *file) 
{
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

void 
CFCUtil_die(const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void 
CFCUtil_warn(const char* format, ...) 
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

void*
CFCUtil_make_perl_obj(void *ptr, const char *klass)
{
    SV *inner_obj = newSV(0);
    SvOBJECT_on(inner_obj);
    PL_sv_objcount++;
    SvUPGRADE(inner_obj, SVt_PVMG);
    sv_setiv(inner_obj, PTR2IV(ptr));

    // Connect class association.
    HV *stash = gv_stashpvn((char*)klass, strlen(klass), TRUE);
    SvSTASH_set(inner_obj, (HV*)SvREFCNT_inc(stash));

    return  inner_obj;
}

