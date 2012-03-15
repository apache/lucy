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
#include <stdio.h>
#include <ctype.h>

#ifndef true
    #define true 1
    #define false 0
#endif

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCFile.h"
#include "CFCUtil.h"
#include "CFCClass.h"

struct CFCFile {
    CFCBase base;
    CFCBase **blocks;
    CFCClass **classes;
    int modified;
    char *source_class;
    char *guard_name;
    char *guard_start;
    char *guard_close;
    char *path_part;
};

const static CFCMeta CFCFILE_META = {
    "Clownfish::CFC::Model::File",
    sizeof(CFCFile),
    (CFCBase_destroy_t)CFCFile_destroy
};

CFCFile*
CFCFile_new(const char *source_class) {

    CFCFile *self = (CFCFile*)CFCBase_allocate(&CFCFILE_META);
    return CFCFile_init(self, source_class);
}

CFCFile*
CFCFile_init(CFCFile *self, const char *source_class) {
    CFCUTIL_NULL_CHECK(source_class);
    self->modified = false;
    self->source_class = CFCUtil_strdup(source_class);
    self->blocks = (CFCBase**)CALLOCATE(1, sizeof(CFCBase*));
    self->classes = (CFCClass**)CALLOCATE(1, sizeof(CFCBase*));

    // Derive include guard name, plus C code for opening and closing the
    // guard.
    size_t len = strlen(source_class);
    self->guard_name = (char*)MALLOCATE(len + sizeof("H_") + 1);
    self->guard_start = (char*)MALLOCATE(len * 2 + 40);
    self->guard_close = (char*)MALLOCATE(len + 20);
    memcpy(self->guard_name, "H_", 2);
    size_t i, j;
    for (i = 0, j = 2; i < len; i++, j++) {
        char c = source_class[i];
        if (c == ':') {
            self->guard_name[j] = '_';
            i++;
        }
        else {
            self->guard_name[j] = toupper(c);
        }
    }
    self->guard_name[j] = '\0';
    sprintf(self->guard_start, "#ifndef %s\n#define %s 1\n", self->guard_name,
            self->guard_name);
    sprintf(self->guard_close, "#endif /* %s */\n", self->guard_name);

    // Cache partial path derived from source_class.
    self->path_part = (char*)MALLOCATE(len + 1);
    for (i = 0, j = 0; i < len; i++, j++) {
        char c = source_class[i];
        if (c == ':') {
            self->path_part[j] = CFCUTIL_PATH_SEP_CHAR;
            i++;
        }
        else {
            self->path_part[j] = c;
        }
    }
    self->path_part[j] = '\0';

    return self;
}

void
CFCFile_destroy(CFCFile *self) {
    size_t i;
    for (i = 0; self->blocks[i] != NULL; i++) {
        CFCBase_decref(self->blocks[i]);
    }
    FREEMEM(self->blocks);
    for (i = 0; self->classes[i] != NULL; i++) {
        CFCBase_decref((CFCBase*)self->classes[i]);
    }
    FREEMEM(self->classes);
    FREEMEM(self->guard_name);
    FREEMEM(self->guard_start);
    FREEMEM(self->guard_close);
    FREEMEM(self->source_class);
    FREEMEM(self->path_part);
    CFCBase_destroy((CFCBase*)self);
}

void
CFCFile_add_block(CFCFile *self, CFCBase *block) {
    CFCUTIL_NULL_CHECK(block);
    const char *cfc_class = CFCBase_get_cfc_class(block);

    // Add to classes array if the block is a CFCClass.
    if (strcmp(cfc_class, "Clownfish::CFC::Model::Class") == 0) {
        size_t num_class_blocks = 0;
        while (self->classes[num_class_blocks] != NULL) {
            num_class_blocks++;
        }
        num_class_blocks++;
        size_t size = (num_class_blocks + 1) * sizeof(CFCClass*);
        self->classes = (CFCClass**)REALLOCATE(self->classes, size);
        self->classes[num_class_blocks - 1]
            = (CFCClass*)CFCBase_incref(block);
        self->classes[num_class_blocks] = NULL;
    }

    // Add to blocks array.
    if (strcmp(cfc_class, "Clownfish::CFC::Model::Class") == 0
        || strcmp(cfc_class, "Clownfish::CFC::Model::Parcel") == 0
        || strcmp(cfc_class, "Clownfish::CFC::Model::CBlock") == 0
       ) {
        size_t num_blocks = 0;
        while (self->blocks[num_blocks] != NULL) {
            num_blocks++;
        }
        num_blocks++;
        size_t size = (num_blocks + 1) * sizeof(CFCBase*);
        self->blocks = (CFCBase**)REALLOCATE(self->blocks, size);
        self->blocks[num_blocks - 1] = CFCBase_incref(block);
        self->blocks[num_blocks] = NULL;
    }
    else {
        CFCUtil_die("Wrong kind of object: '%s'", cfc_class);
    }
}

static void
S_some_path(CFCFile *self, char *buf, size_t buf_size, const char *base_dir,
            const char *ext) {
    size_t needed = CFCFile_path_buf_size(self, base_dir);
    if (strlen(ext) > 4) {
        CFCUtil_die("ext cannot be more than 4 characters.");
    }
    if (needed > buf_size) {
        CFCUtil_die("Need buf_size of %lu, but got %lu",
                    (unsigned long)needed, (unsigned long)buf_size);
    }
    if (base_dir) {
        sprintf(buf, "%s" CFCUTIL_PATH_SEP "%s%s", base_dir, self->path_part,
                ext);
    }
    else {
        sprintf(buf, "%s%s", self->path_part, ext);
    }
    size_t i;
    for (i = 0; buf[i] != '\0'; i++) {
        #ifdef _WIN32
        if (buf[i] == '/') { buf[i] = '\\'; }
        #else
        if (buf[i] == '\\') { buf[i] = '/'; }
        #endif
    }
}

size_t
CFCFile_path_buf_size(CFCFile *self, const char *base_dir) {
    size_t size = strlen(self->path_part);
    size += 4; // Max extension length.
    size += 1; // NULL-termination.
    if (base_dir) {
        size += strlen(base_dir);
        size += strlen(CFCUTIL_PATH_SEP);
    }
    return size;
}

void
CFCFile_c_path(CFCFile *self, char *buf, size_t buf_size,
               const char *base_dir) {
    S_some_path(self, buf, buf_size, base_dir, ".c");
}

void
CFCFile_h_path(CFCFile *self, char *buf, size_t buf_size,
               const char *base_dir) {
    S_some_path(self, buf, buf_size, base_dir, ".h");
}

void
CFCFile_cfh_path(CFCFile *self, char *buf, size_t buf_size,
                 const char *base_dir) {
    S_some_path(self, buf, buf_size, base_dir, ".cfh");
}

CFCBase**
CFCFile_blocks(CFCFile *self) {
    return self->blocks;
}

CFCClass**
CFCFile_classes(CFCFile *self) {
    return self->classes;
}

void
CFCFile_set_modified(CFCFile *self, int modified) {
    self->modified = !!modified;
}

int
CFCFile_get_modified(CFCFile *self) {
    return self->modified;
}

const char*
CFCFile_get_source_class(CFCFile *self) {
    return self->source_class;
}

const char*
CFCFile_guard_name(CFCFile *self) {
    return self->guard_name;
}

const char*
CFCFile_guard_start(CFCFile *self) {
    return self->guard_start;
}

const char*
CFCFile_guard_close(CFCFile *self) {
    return self->guard_close;
}

