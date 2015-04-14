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
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Document/Doc.h"
#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Hash.h"
#include "Clownfish/Class.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Freezer.h"

Doc*
Doc_init(Doc *self, void *fields, int32_t doc_id) {
    DocIVARS *const ivars = Doc_IVARS(self);
    Hash *hash;

    if (fields) {
        hash = (Hash *)INCREF(CERTIFY(fields, HASH));
    }
    else {
        hash = Hash_new(0);
    }
    ivars->fields = hash;
    ivars->doc_id = doc_id;

    return self;
}

void
Doc_Set_Fields_IMP(Doc *self, void *fields) {
    DocIVARS *const ivars = Doc_IVARS(self);
    DECREF(ivars->fields);
    ivars->fields = CERTIFY(fields, HASH);
}

uint32_t
Doc_Get_Size_IMP(Doc *self) {
    Hash *hash = (Hash*)Doc_IVARS(self)->fields;
    return Hash_Get_Size(hash);
}

void
Doc_Store_IMP(Doc *self, String *field, Obj *value) {
    Hash *hash = (Hash*)Doc_IVARS(self)->fields;
    Hash_Store(hash, field, INCREF(value));
}

void
Doc_Serialize_IMP(Doc *self, OutStream *outstream) {
    DocIVARS *const ivars = Doc_IVARS(self);
    Hash *hash = (Hash*)ivars->fields;
    Freezer_serialize_hash(hash, outstream);
    OutStream_Write_C32(outstream, ivars->doc_id);
}

Doc*
Doc_Deserialize_IMP(Doc *self, InStream *instream) {
    DocIVARS *const ivars = Doc_IVARS(self);
    ivars->fields = Freezer_read_hash(instream);
    ivars->doc_id = InStream_Read_C32(instream);
    return self;
}

Obj*
Doc_Extract_IMP(Doc *self, String *field) {
    Hash *hash = (Hash*)Doc_IVARS(self)->fields;
    return INCREF(Hash_Fetch(hash, field));
}

void*
Doc_To_Host_IMP(Doc *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(void*);
}

Hash*
Doc_Dump_IMP(Doc *self) {
    UNUSED_VAR(self);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Hash*);
}

Doc*
Doc_Load_IMP(Doc *self, Obj *dump) {
    UNUSED_VAR(self);
    UNUSED_VAR(dump);
    THROW(ERR, "TODO");
    UNREACHABLE_RETURN(Doc*);
}

bool
Doc_Equals_IMP(Doc *self, Obj *other) {
    if ((Doc*)other == self)   { return true;  }
    if (!Obj_Is_A(other, DOC)) { return false; }
    DocIVARS *const ivars = Doc_IVARS(self);
    DocIVARS *const ovars = Doc_IVARS((Doc*)other);
    return Hash_Equals((Hash*)ivars->fields, (Obj*)ovars->fields);
}

void
Doc_Destroy_IMP(Doc *self) {
    DocIVARS *const ivars = Doc_IVARS(self);
    DECREF(ivars->fields);
    SUPER_DESTROY(self, DOC);
}


