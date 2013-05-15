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

#define C_LUCY_DOC
#define CHY_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Document/Doc.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/VTable.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

Doc*
Doc_init(Doc *self, void *fields, int32_t doc_id) {
    Hash *hash;

    if (fields) {
        hash = (Hash *)CERTIFY(fields, HASH);
        INCREF(hash);
    }
    else {
        hash = Hash_new(0);
    }
    self->fields = hash;
    self->doc_id = doc_id;

    return self;
}

void
Doc_set_fields(Doc *self, void *fields) {
    DECREF(self->fields);
    self->fields = CERTIFY(fields, HASH);
}

uint32_t
Doc_get_size(Doc *self) {
    Hash *hash = (Hash *)self->fields;
    return Hash_Get_Size(hash);
}

void
Doc_store(Doc *self, const CharBuf *field, Obj *value) {
    Hash *hash = (Hash *)self->fields;
    Hash_Store(hash, (Obj *)field, value);
    INCREF(value);
}

void
Doc_serialize(Doc *self, OutStream *outstream) {
    Hash *hash = (Hash *)self->fields;
    Hash_Serialize(hash, outstream);
    OutStream_Write_C32(outstream, self->doc_id);
}

Doc*
Doc_deserialize(Doc *self, InStream *instream) {
     Hash *hash = (Hash*)VTable_Make_Obj(HASH);
     self->fields = Hash_Deserialize(hash, instream);
     self->doc_id = InStream_Read_C32(instream);
     return self;
}

Obj*
Doc_extract(Doc *self, CharBuf *field,
                 ViewCharBuf *target) {
    Hash *hash = (Hash *)self->fields;
    Obj  *obj  = Hash_Fetch(hash, (Obj *)field);

    if (obj && Obj_Is_A(obj, CHARBUF)) {
        ViewCB_Assign(target, (CharBuf *)obj);
    }

    return obj;
}

void*
Doc_to_host(Doc *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

Hash*
Doc_dump(Doc *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Hash*);
}

Doc*
Doc_load(Doc *self, Obj *dump) {
    UNUSED_VAR(self);
    UNUSED_VAR(dump);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Doc*);
}

bool
Doc_equals(Doc *self, Obj *other) {
    Doc *twin = (Doc*)other;

    if (twin == self)                    { return true;  }
    if (!Obj_Is_A(other, DOC)) { return false; }

    return Hash_Equals((Hash*)self->fields, (Obj*)twin->fields);
}

void
Doc_destroy(Doc *self) {
    DECREF(self->fields);
    SUPER_DESTROY(self, DOC);
}


