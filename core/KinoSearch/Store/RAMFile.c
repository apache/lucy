#define C_KINO_RAMFILE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Store/RAMFile.h"

RAMFile*
RAMFile_new(ByteBuf *contents, bool_t read_only)
{
    RAMFile *self = (RAMFile*)VTable_Make_Obj(RAMFILE);
    return RAMFile_init(self, contents, read_only);
}

RAMFile*
RAMFile_init(RAMFile *self, ByteBuf *contents, bool_t read_only)
{
    self->contents = contents ? (ByteBuf*)INCREF(contents) : BB_new(0);
    self->read_only = read_only;
    return self;
}

void
RAMFile_destroy(RAMFile *self)
{
    DECREF(self->contents);
    SUPER_DESTROY(self, RAMFILE);
}

ByteBuf*
RAMFile_get_contents(RAMFile *self) { return self->contents; }
bool_t
RAMFile_read_only(RAMFile *self) { return self->read_only; }
void
RAMFile_set_read_only(RAMFile *self, bool_t read_only)
    { self->read_only = read_only; }

/* Copyright 2009-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

