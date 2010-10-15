#include <stdarg.h>

#define C_KINO_FILEHANDLE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Store/FileHandle.h"

int32_t FH_object_count = 0;

FileHandle*
FH_do_open(FileHandle *self, const CharBuf *path, uint32_t flags)
{
    self->path    = path ? CB_Clone(path) : CB_new(0);
    self->flags   = flags;

    // Track number of live FileHandles released into the wild. 
    FH_object_count++;

    ABSTRACT_CLASS_CHECK(self, FILEHANDLE);
    return self;
}

void
FH_destroy(FileHandle *self)
{
    FH_Close(self);
    DECREF(self->path);
    SUPER_DESTROY(self, FILEHANDLE);

    // Decrement count of FileHandle objects in existence. 
    FH_object_count--;
}

bool_t
FH_grow(FileHandle *self, int64_t length)
{
    UNUSED_VAR(self);
    UNUSED_VAR(length);
    return true;
}

void
FH_set_path(FileHandle *self, const CharBuf *path)
{
    CB_Mimic(self->path, (Obj*)path);
}

CharBuf*
FH_get_path(FileHandle *self) { return self->path; }


