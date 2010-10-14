#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Util/Json.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Store/Folder.h"

bool_t
Json_spew_json(Obj *dump, Folder *folder, const CharBuf *path)
{
    bool_t result = (bool_t)Host_callback_i64(JSON, "spew_json", 3, 
        ARG_OBJ("dump", dump), ARG_OBJ("folder", folder), 
        ARG_STR("path", path));
    if (!result) { ERR_ADD_FRAME(Err_get_error()); }
    return result;
}

Obj*
Json_slurp_json(Folder *folder, const CharBuf *path)
{
    Obj *dump = Host_callback_obj(JSON, "slurp_json", 2, 
        ARG_OBJ("folder", folder), ARG_STR("path", path));
    if (!dump) { ERR_ADD_FRAME(Err_get_error()); }
    return dump;
}

CharBuf*
Json_to_json(Obj *dump)
{
    return Host_callback_str(JSON, "to_json", 1,
        ARG_OBJ("dump", dump));
}

Obj*
Json_from_json(CharBuf *json)
{
    return Host_callback_obj(JSON, "from_json", 1, 
        ARG_STR("json", json));
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

