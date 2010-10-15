#define C_KINO_OBJ
#define C_KINO_VTABLE
#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "KinoSearch/Object/Obj.h"
#include "KinoSearch/Object/CharBuf.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Object/Hash.h"
#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/Memory.h"

Obj*
Obj_init(Obj *self)
{
    ABSTRACT_CLASS_CHECK(self, OBJ);
    return self;
}

void
Obj_destroy(Obj *self)
{
    FREEMEM(self);
}

int32_t
Obj_hash_sum(Obj *self)
{
    int64_t hash_sum = PTR_TO_I64(self);
    return (int32_t)hash_sum;
}

bool_t
Obj_is_a(Obj *self, VTable *ancestor)
{
    VTable *vtable = self ? self->vtable : NULL;

    while (vtable != NULL) {
        if (vtable == ancestor)
            return true;
        vtable = vtable->parent;
    }

    return false;
}

bool_t
Obj_equals(Obj *self, Obj *other)
{
    return (self == other);
}

void
Obj_serialize(Obj *self, OutStream *outstream)
{
    CharBuf *class_name = Obj_Get_Class_Name(self);
    CB_Serialize(class_name, outstream);
}

Obj*
Obj_deserialize(Obj *self, InStream *instream)
{
    CharBuf *class_name = CB_deserialize(NULL, instream);
    if (!self) {
        VTable *vtable = VTable_singleton(class_name, OBJ);
        self = VTable_Make_Obj(vtable);
    }
    else {
        CharBuf *my_class = VTable_Get_Name(self->vtable);
        if (!CB_Equals(class_name, (Obj*)my_class)) 
            THROW(ERR, "Class mismatch: %o %o", class_name, my_class);
    }
    DECREF(class_name);
    return Obj_init(self);
}

CharBuf*
Obj_to_string(Obj *self)
{
#if (SIZEOF_PTR == 4)
    return CB_newf("%o@0x%x32", Obj_Get_Class_Name(self), self);
#elif (SIZEOF_PTR == 8)
    int64_t   iaddress   = PTR_TO_I64(self);
    uint64_t  address    = (uint64_t)iaddress;
    uint32_t  address_hi = address >> 32;
    uint32_t  address_lo = address & 0xFFFFFFFF;
    return CB_newf("%o@0x%x32%x32", Obj_Get_Class_Name(self), address_hi,
        address_lo);
#else
  #error Unexpected pointer size.
#endif
}

Obj*
Obj_dump(Obj *self)
{
    return (Obj*)Obj_To_String(self);
}

VTable*
Obj_get_vtable(Obj *self) { return self->vtable; }
CharBuf*
Obj_get_class_name(Obj *self) { return VTable_Get_Name(self->vtable); }


