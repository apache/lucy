#define C_LUCY_BLOBTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/BlobType.h"
#include "Lucy/Index/Similarity.h"

BlobType*
BlobType_new(Similarity *similarity)
{
    BlobType *self = (BlobType*)VTable_Make_Obj(BLOBTYPE);
    return BlobType_init(self, similarity);
}

BlobType*
BlobType_init(BlobType *self, Similarity *similarity)
{
    BlobType_init2(self, similarity, true, false);
    return self;
}

BlobType*
BlobType_init2(BlobType *self, Similarity *similarity, bool_t stored, 
               bool_t sortable)
{
    FType_init2((FieldType*)self, similarity, 1.0, false, stored, sortable);
    return self;
}

bool_t
BlobType_binary(BlobType *self)
{
    UNUSED_VAR(self);
    return true;
}

uint8_t
BlobType_scalar_type_id(BlobType *self)
{
    UNUSED_VAR(self);
    return Obj_BLOB;
}

bool_t
BlobType_equals(BlobType *self, Obj *other)
{
    BlobType *evil_twin = (BlobType*)other;
    if (evil_twin == self) return true;
    if (!Obj_Is_A(other, BLOBTYPE)) return false;
    return FType_equals((FieldType*)self, other);
}

Hash*
BlobType_dump_for_schema(BlobType *self) 
{
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
    if (!self->stored) {
        Hash_Store_Str(dump, "stored", 6, (Obj*)CB_newf("0"));
    }
    if (self->sortable) {
        Hash_Store_Str(dump, "sortable", 8, (Obj*)CB_newf("1"));
    }

    return dump;
}

Hash*
BlobType_dump(BlobType *self)
{
    Hash *dump = BlobType_Dump_For_Schema(self);
    Hash_Store_Str(dump, "_class", 6, 
        (Obj*)CB_Clone(BlobType_Get_Class_Name(self)));
    if (self->sim) {
        Hash_Store_Str(dump, "similarity", 10, (Obj*)Sim_Dump(self->sim));
    }
    DECREF(Hash_Delete_Str(dump, "type", 4));
    return dump;
}

BlobType*
BlobType_load(BlobType *self, Obj *dump)
{
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    VTable *vtable 
        = (class_name != NULL && Obj_Is_A((Obj*)class_name, CHARBUF)) 
        ? VTable_singleton(class_name, NULL)
        : BLOBTYPE;
    BlobType *loaded = (BlobType*)VTable_Make_Obj(vtable);

    // Extract boost.
    Obj *boost_dump = Hash_Fetch_Str(source, "boost", 5);
    float boost = boost_dump ? (float)Obj_To_F64(boost_dump) : 1.0f;

    // Find boolean properties.
    Obj *indexed_dump = Hash_Fetch_Str(source, "indexed", 7);
    Obj *stored_dump  = Hash_Fetch_Str(source, "stored", 6);
    Obj *sort_dump    = Hash_Fetch_Str(source, "sortable", 8);
    bool_t indexed  = indexed_dump ? (bool_t)Obj_To_I64(indexed_dump) : false;
    bool_t stored   = stored_dump  ? (bool_t)Obj_To_I64(stored_dump)  : true;
    bool_t sortable = sort_dump    ? (bool_t)Obj_To_I64(sort_dump)    : false;

    // Extract a Similarity.
    Similarity *sim = NULL;
    Obj *sim_dump = Hash_Fetch_Str(source, "similarity", 10);
    if (sim_dump) {
        if (Obj_Is_A(sim_dump, SIMILARITY)) {
            // Schema munged the dump and installed a shared sim.
            sim = (Similarity*)INCREF(sim_dump);
        }
        else {
            sim = (Similarity*)CERTIFY(
                Obj_Load(sim_dump, sim_dump), SIMILARITY);
        }
    }

    BlobType_init2(loaded, sim, stored, sortable);
    loaded->boost    = boost;
    loaded->indexed  = indexed;

    return loaded;
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

