#define C_KINO_MOCKFILEHANDLE
#define C_KINO_FILEWINDOW
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test/Store/MockFileHandle.h"
#include "KinoSearch/Store/FileWindow.h"

MockFileHandle*
MockFileHandle_new(const CharBuf *path, int64_t length) 
{
    MockFileHandle *self = (MockFileHandle*)VTable_Make_Obj(MOCKFILEHANDLE);
    return MockFileHandle_init(self, path, length);
}

MockFileHandle*
MockFileHandle_init(MockFileHandle *self, const CharBuf *path, int64_t length) 
{
    FH_do_open((FileHandle*)self, path, 0);
    self->len = length;
    return self;
}

bool_t
MockFileHandle_window(MockFileHandle *self, FileWindow *window, int64_t offset, 
                     int64_t len)
{
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, offset, len);
    return true;
}

bool_t
MockFileHandle_release_window(MockFileHandle *self, FileWindow *window)
{
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

int64_t
MockFileHandle_length(MockFileHandle *self)
{
    return self->len;
}

bool_t
MockFileHandle_close(MockFileHandle *self)
{
    UNUSED_VAR(self);
    return true;
}

/* Copyright 2009-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

