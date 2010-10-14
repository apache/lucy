#define C_KINO_RAMFOLDER
#define C_KINO_RAMDIRHANDLE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Store/RAMDirHandle.h"
#include "KinoSearch/Store/RAMFolder.h"
#include "KinoSearch/Util/IndexFileNames.h"

RAMDirHandle*
RAMDH_new(RAMFolder *folder)
{
    RAMDirHandle *self = (RAMDirHandle*)VTable_Make_Obj(RAMDIRHANDLE);
    return RAMDH_init(self, folder);
}

RAMDirHandle*
RAMDH_init(RAMDirHandle *self, RAMFolder *folder)
{
    DH_init((DirHandle*)self, RAMFolder_Get_Path(folder));
    self->folder = (RAMFolder*)INCREF(folder);
    self->elems  = Hash_Keys(self->folder->entries);
    self->tick   = -1;
    return self;
}

bool_t
RAMDH_close(RAMDirHandle *self)
{
    if (self->elems) {
        VA_Dec_RefCount(self->elems);
        self->elems = NULL;
    }
    if (self->folder) {
        RAMFolder_Dec_RefCount(self->folder);
        self->folder = NULL;
    }
    return true;
}

bool_t
RAMDH_next(RAMDirHandle *self)
{
    if (self->elems) {
        self->tick++;
        if (self->tick < (int32_t)VA_Get_Size(self->elems)) {
            CharBuf *path = (CharBuf*)CERTIFY(
                VA_Fetch(self->elems, self->tick), CHARBUF);
            CB_Mimic(self->entry, (Obj*)path);
            return true;
        }
        else {
            self->tick--;
            return false;
        }
    }
    return false;
}

bool_t
RAMDH_entry_is_dir(RAMDirHandle *self)
{
    if (self->elems) {
        CharBuf *name = (CharBuf*)VA_Fetch(self->elems, self->tick);
        if (name) {
            return RAMFolder_Local_Is_Directory(self->folder, name);
        }
    }
    return false;
}

bool_t
RAMDH_entry_is_symlink(RAMDirHandle *self)
{
    UNUSED_VAR(self);
    return false;
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

