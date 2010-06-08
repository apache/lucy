#define C_LUCY_SIMILARITY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Index/Similarity.h"

#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

Similarity*
Sim_init(Similarity *self) 
{
    ABSTRACT_CLASS_CHECK(self, SIMILARITY);
    return self;
}

Obj*
Sim_dump(Similarity *self)
{
    Hash *dump = Hash_new(0);
    Hash_Store_Str(dump, "_class", 6, 
        (Obj*)CB_Clone(Sim_Get_Class_Name(self)));
    return (Obj*)dump;
}

Similarity*
Sim_load(Similarity *self, Obj *dump)
{
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)CERTIFY(
        Hash_Fetch_Str(source, "_class", 6), CHARBUF);
    VTable *vtable = VTable_singleton(class_name, NULL);
    Similarity *loaded = (Similarity*)VTable_Make_Obj(vtable);
    UNUSED_VAR(self);
    return Sim_init(loaded);
}

void
Sim_serialize(Similarity *self, OutStream *target)
{
    // Only the class name. 
    CB_Serialize(Sim_Get_Class_Name(self), target);
}

Similarity*
Sim_deserialize(Similarity *self, InStream *instream)
{
    CharBuf *class_name = CB_deserialize(NULL, instream);
    if (!self) {
        VTable *vtable = VTable_singleton(class_name, SIMILARITY);
        self = (Similarity*)VTable_Make_Obj(vtable);
    }
    else if (!CB_Equals(class_name, (Obj*)Sim_Get_Class_Name(self))) {
        THROW(ERR, "Class name mismatch: '%o' '%o'", Sim_Get_Class_Name(self),
            class_name);
    }
    DECREF(class_name);

    Sim_init(self);
    return self;
}

bool_t
Sim_equals(Similarity *self, Obj *other)
{
    if (Sim_Get_VTable(self) != Obj_Get_VTable(other)) return false;
    return true;
}

/* Copyright 2010 The Apache Software Foundation
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

