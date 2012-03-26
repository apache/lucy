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

#define C_LUCY_BLOBTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/BlobType.h"

BlobType*
BlobType_new(bool_t stored) {
    BlobType *self = (BlobType*)VTable_Make_Obj(BLOBTYPE);
    return BlobType_init(self, stored);
}

BlobType*
BlobType_init(BlobType *self, bool_t stored) {
    FType_init((FieldType*)self);
    self->stored = stored;
    return self;
}

bool_t
BlobType_binary(BlobType *self) {
    UNUSED_VAR(self);
    return true;
}

void
BlobType_set_sortable(BlobType *self, bool_t sortable) {
    UNUSED_VAR(self);
    if (sortable) { THROW(ERR, "BlobType fields can't be sortable"); }
}

ViewByteBuf*
BlobType_make_blank(BlobType *self) {
    UNUSED_VAR(self);
    return ViewBB_new(NULL, 0);
}

int8_t
BlobType_primitive_id(BlobType *self) {
    UNUSED_VAR(self);
    return FType_BLOB;
}

bool_t
BlobType_equals(BlobType *self, Obj *other) {
    BlobType *twin = (BlobType*)other;
    if (twin == self)               { return true; }
    if (!Obj_Is_A(other, BLOBTYPE)) { return false; }
    return FType_equals((FieldType*)self, other);
}

Hash*
BlobType_dump_for_schema(BlobType *self) {
    Hash *dump = Hash_new(0);
    Hash_Store_Str(dump, "type", 4, (Obj*)CB_newf("blob"));

    // Store attributes that override the defaults -- even if they're
    // meaningless.
    if (self->boost != 1.0) {
        Hash_Store_Str(dump, "boost", 5, (Obj*)CB_newf("%f64", self->boost));
    }
    if (self->indexed) {
        Hash_Store_Str(dump, "indexed", 7, (Obj*)CB_newf("1"));
    }
    if (self->stored) {
        Hash_Store_Str(dump, "stored", 6, (Obj*)CB_newf("1"));
    }

    return dump;
}

Hash*
BlobType_dump(BlobType *self) {
    Hash *dump = BlobType_Dump_For_Schema(self);
    Hash_Store_Str(dump, "_class", 6,
                   (Obj*)CB_Clone(BlobType_Get_Class_Name(self)));
    DECREF(Hash_Delete_Str(dump, "type", 4));
    return dump;
}

BlobType*
BlobType_load(BlobType *self, Obj *dump) {
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    VTable *vtable
        = (class_name != NULL && Obj_Is_A((Obj*)class_name, CHARBUF))
          ? VTable_singleton(class_name, NULL)
          : BLOBTYPE;
    BlobType *loaded     = (BlobType*)VTable_Make_Obj(vtable);
    Obj *boost_dump      = Hash_Fetch_Str(source, "boost", 5);
    Obj *indexed_dump    = Hash_Fetch_Str(source, "indexed", 7);
    Obj *stored_dump     = Hash_Fetch_Str(source, "stored", 6);
    UNUSED_VAR(self);

    BlobType_init(loaded, false);
    if (boost_dump)   { loaded->boost   = (float)Obj_To_F64(boost_dump);    }
    if (indexed_dump) { loaded->indexed = (bool_t)Obj_To_I64(indexed_dump); }
    if (stored_dump)  { loaded->stored  = (bool_t)Obj_To_I64(stored_dump);  }

    return loaded;
}


