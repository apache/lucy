/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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


