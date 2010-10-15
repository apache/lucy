#define C_KINO_FREEZER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Util/Freezer.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"

void
Freezer_freeze(Obj *obj, OutStream *outstream)
{
    CB_Serialize(Obj_Get_Class_Name(obj), outstream);
    Obj_Serialize(obj, outstream);
}

Obj*
Freezer_thaw(InStream *instream)
{
    CharBuf *class_name = CB_deserialize(NULL, instream);
    VTable *vtable = VTable_singleton(class_name, NULL);
    Obj *blank = VTable_Make_Obj(vtable);
    DECREF(class_name);
    return Obj_Deserialize(blank, instream);
}


