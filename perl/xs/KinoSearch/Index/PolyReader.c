#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/PolyReader.h"
#include "KinoSearch/Index/Snapshot.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Store/Folder.h"

Obj*
PolyReader_try_open_segreaders(PolyReader *self, VArray *segments)
{
    return Host_callback_obj(self, "try_open_segreaders", 1, 
        ARG_OBJ("segments", segments));
}

CharBuf*
PolyReader_try_read_snapshot(Snapshot *snapshot, Folder *folder, 
                             const CharBuf *path) 
{
    return (CharBuf*)Host_callback_obj(POLYREADER, "try_read_snapshot", 3,
        ARG_OBJ("snapshot", snapshot), ARG_OBJ("folder", folder), 
        ARG_STR("path", path));
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

