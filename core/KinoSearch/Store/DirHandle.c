#define C_KINO_DIRHANDLE
#include <stdarg.h>
#include "KinoSearch/Util/ToolSet.h"
#include "KinoSearch/Store/DirHandle.h"

DirHandle*
DH_init(DirHandle *self, const CharBuf *dir)
{
    self->dir   = CB_Clone(dir);
    self->entry = CB_new(32);
    ABSTRACT_CLASS_CHECK(self, DIRHANDLE);
    return self;
}

void
DH_destroy(DirHandle *self)
{
    DH_Close(self);
    DECREF(self->dir);
    DECREF(self->entry);
    SUPER_DESTROY(self, DIRHANDLE);
}

CharBuf*
DH_get_dir(DirHandle *self)   { return self->dir; }
CharBuf*
DH_get_entry(DirHandle *self) { return self->entry; }


