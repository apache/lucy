#define C_KINO_FILEWINDOW
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Store/FileWindow.h"

FileWindow*
FileWindow_new()
{
    FileWindow *self = (FileWindow*)VTable_Make_Obj(FILEWINDOW);
    return FileWindow_init(self);
}

FileWindow*
FileWindow_init(FileWindow *self)
{
    return self;
}

void
FileWindow_set_offset(FileWindow *self, int64_t offset)
{
    if (self->buf != NULL) {
        if (offset != self->offset) {
            THROW(ERR, "Can't set offset to %i64 instead of %i64 unless buf "
                "is NULL", offset, self->offset);
        }
    }
    self->offset = offset;
}

void
FileWindow_set_window(FileWindow *self, char *buf, int64_t offset, int64_t len)
{
    self->buf    = buf;
    self->offset = offset;
    self->len    = len;
}


