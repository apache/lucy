#define C_LUCY_OBJ
#define C_LUCY_VTABLE
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Lucy/Object/Obj.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/Hash.h"
#include "Lucy/Object/VTable.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Memory.h"

Obj*
Obj_init(Obj *self)
{
    ABSTRACT_CLASS_CHECK(self, OBJ);
    return self;
}

void
Obj_destroy(Obj *self)
{
    Obj_Dec_RefCount(self->vtable);
    FREEMEM(self);
}

i32_t
Obj_hash_code(Obj *self)
{
    return (i32_t)self;
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
    size_t address = self;
    u32_t  address_hi = address >> 32;
    u32_t  address_lo = address & 0xFFFFFFFF;
    return CB_newf("%o@0x%x32%x32", Obj_Get_Class_Name(self), address_hi,
        address_lo);
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

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

