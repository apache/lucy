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

#define C_LUCY_RAMFILEHANDLE
#define C_LUCY_RAMFILE
#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/FileWindow.h"

RAMFileHandle*
RAMFH_open(const CharBuf *path, uint32_t flags, RAMFile *file) {
    RAMFileHandle *self = (RAMFileHandle*)VTable_Make_Obj(RAMFILEHANDLE);
    return RAMFH_do_open(self, path, flags, file);
}

RAMFileHandle*
RAMFH_do_open(RAMFileHandle *self, const CharBuf *path, uint32_t flags,
              RAMFile *file) {
    bool_t must_create
        = (flags & (FH_CREATE | FH_EXCLUSIVE)) == (FH_CREATE | FH_EXCLUSIVE)
          ? true : false;
    bool_t can_create
        = (flags & (FH_CREATE | FH_WRITE_ONLY)) == (FH_CREATE | FH_WRITE_ONLY)
          ? true : false;

    FH_do_open((FileHandle*)self, path, flags);

    // Obtain a RAMFile.
    if (file) {
        if (must_create) {
            Err_set_error(Err_new(CB_newf("File '%o' exists, but FH_EXCLUSIVE flag supplied", path)));
            DECREF(self);
            return NULL;
        }
        self->ram_file = (RAMFile*)INCREF(file);
    }
    else if (can_create) {
        self->ram_file = RAMFile_new(NULL, false);
    }
    else {
        Err_set_error(Err_new(CB_newf("Must supply either RAMFile or FH_CREATE | FH_WRITE_ONLY")));
        DECREF(self);
        return NULL;
    }

    // Prevent writes to to the RAMFile if FH_READ_ONLY was specified.
    if (flags & FH_READ_ONLY) {
        RAMFile_Set_Read_Only(self->ram_file, true);
    }

    self->len = BB_Get_Size(self->ram_file->contents);

    return self;
}

void
RAMFH_destroy(RAMFileHandle *self) {
    DECREF(self->ram_file);
    SUPER_DESTROY(self, RAMFILEHANDLE);
}

bool_t
RAMFH_window(RAMFileHandle *self, FileWindow *window, int64_t offset,
             int64_t len) {
    int64_t end = offset + len;
    if (!(self->flags & FH_READ_ONLY)) {
        Err_set_error(Err_new(CB_newf("Can't read from write-only handle")));
        return false;
    }
    else if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from negative offset %i64",
                                      offset)));
        return false;
    }
    else if (end > self->len) {
        Err_set_error(Err_new(CB_newf("Tried to read past EOF: offset %i64 + request %i64 > len %i64",
                                      offset, len, self->len)));
        return false;
    }
    else {
        char *const buf = BB_Get_Buf(self->ram_file->contents) + offset;
        FileWindow_Set_Window(window, buf, offset, len);
        return true;
    }
}

bool_t
RAMFH_release_window(RAMFileHandle *self, FileWindow *window) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

bool_t
RAMFH_read(RAMFileHandle *self, char *dest, int64_t offset, size_t len) {
    int64_t end = offset + len;
    if (!(self->flags & FH_READ_ONLY)) {
        Err_set_error(Err_new(CB_newf("Can't read from write-only handle")));
        return false;
    }
    else if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from a negative offset %i64",
                                      offset)));
        return false;
    }
    else if (end > self->len) {
        Err_set_error(Err_new(CB_newf("Attempt to read %u64 bytes starting at %i64 goes past EOF %u64",
                                      (uint64_t)len, offset, self->len)));
        return false;
    }
    else {
        char *const source = BB_Get_Buf(self->ram_file->contents) + offset;
        memcpy(dest, source, len);
        return true;
    }
}

bool_t
RAMFH_write(RAMFileHandle *self, const void *data, size_t len) {
    if (self->ram_file->read_only) {
        Err_set_error(Err_new(CB_newf("Attempt to write to read-only RAMFile")));
        return false;
    }
    BB_Cat_Bytes(self->ram_file->contents, data, len);
    self->len += len;
    return true;
}

bool_t
RAMFH_grow(RAMFileHandle *self, int64_t len) {
    if (len > I32_MAX) {
        Err_set_error(Err_new(CB_newf("Can't support RAM files of size %i64 (> %i32)",
                                      len, (int32_t)I32_MAX)));
        return false;
    }
    else if (self->ram_file->read_only) {
        Err_set_error(Err_new(CB_newf("Can't grow read-only RAMFile '%o'",
                                      self->path)));
        return false;
    }
    else {
        BB_Grow(self->ram_file->contents, (size_t)len);
        return true;
    }
}

RAMFile*
RAMFH_get_file(RAMFileHandle *self) {
    return self->ram_file;
}

int64_t
RAMFH_length(RAMFileHandle *self) {
    return self->len;
}

bool_t
RAMFH_close(RAMFileHandle *self) {
    UNUSED_VAR(self);
    return true;
}


