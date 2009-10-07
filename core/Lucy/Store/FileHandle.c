#include <stdarg.h>

#define C_LUCY_FILEHANDLE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Store/FileHandle.h"

i32_t FH_object_count = 0;

FileHandle*
FH_init(FileHandle *self, const CharBuf *path, u32_t flags)
{
    self->path    = path ? CB_Clone(path) : CB_new(0);
    self->error   = NULL;
    self->flags   = flags;

    /* Track number of live FileHandles released into the wild. */
    FH_object_count++;

    ABSTRACT_CLASS_CHECK(self, FILEHANDLE);
    return self;
}

void
FH_destroy(FileHandle *self)
{
    bool_t success = FH_Close(self);
    CharBuf *error  = NULL;
    if (!success) {
        error = CB_Clone(self->error);
    }

    /* Decrement count of FileHandle objects in existence. */
    FH_object_count--;

    DECREF(self->path);
    DECREF(self->error);
    SUPER_DESTROY(self, FILEHANDLE);

    if (!success) { Err_throw_mess(ERR, error); }
}

void
FH_grow(FileHandle *self, i64_t length)
{
    UNUSED_VAR(self);
    UNUSED_VAR(length);
}

void
FH_set_path(FileHandle *self, const CharBuf *path)
{
    CB_Mimic(self->path, (Obj*)path);
}

CharBuf*
FH_get_path(FileHandle *self) { return self->path; }
CharBuf*
FH_get_error(FileHandle *self) { return self->error; }

void
FH_set_error(FileHandle *self, CharBuf *error)
{
    DECREF(self->error);
    self->error = error ? CB_Clone(error) : NULL;
}

void
FH_setf_error(void *vself, const char *pattern, ...)
{
    va_list args;
    va_start(args, pattern);
    FH_VSetF_Error(vself, pattern, args);
    va_end(args);
}

void
FH_vsetf_error(FileHandle *self, const char *pattern, va_list args)
{
    DECREF(self->error);
    self->error = CB_new(50);
    CB_VCatF(self->error, pattern, args);
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

