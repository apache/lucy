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

#define C_LUCY_NUMERICTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Plan/NumericType.h"

NumericType*
NumType_init(NumericType *self) {
    return NumType_init2(self, 1.0, true, true, false);
}

NumericType*
NumType_init2(NumericType *self, float boost, bool_t indexed, bool_t stored,
              bool_t sortable) {
    FType_init((FieldType*)self);
    self->boost      = boost;
    self->indexed    = indexed;
    self->stored     = stored;
    self->sortable   = sortable;
    return self;
}

bool_t
NumType_binary(NumericType *self) {
    UNUSED_VAR(self);
    return true;
}

Hash*
NumType_dump_for_schema(NumericType *self) {
    Hash *dump = Hash_new(0);
    Hash_Store_Str(dump, "type", 4, (Obj*)NumType_Specifier(self));

    // Store attributes that override the defaults.
    if (self->boost != 1.0) {
        Hash_Store_Str(dump, "boost", 5, (Obj*)CB_newf("%f64", self->boost));
    }
    if (!self->indexed) {
        Hash_Store_Str(dump, "indexed", 7, (Obj*)CFISH_FALSE);
    }
    if (!self->stored) {
        Hash_Store_Str(dump, "stored", 6, (Obj*)CFISH_FALSE);
    }
    if (self->sortable) {
        Hash_Store_Str(dump, "sortable", 8, (Obj*)CFISH_TRUE);
    }

    return dump;
}

Hash*
NumType_dump(NumericType *self) {
    Hash *dump = NumType_Dump_For_Schema(self);
    Hash_Store_Str(dump, "_class", 6,
                   (Obj*)CB_Clone(NumType_Get_Class_Name(self)));
    DECREF(Hash_Delete_Str(dump, "type", 4));
    return dump;
}

NumericType*
NumType_load(NumericType *self, Obj *dump) {
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);

    // Get a VTable
    CharBuf *class_name = (CharBuf*)Hash_Fetch_Str(source, "_class", 6);
    CharBuf *type_spec  = (CharBuf*)Hash_Fetch_Str(source, "type", 4);
    VTable *vtable = NULL;
    if (class_name != NULL && Obj_Is_A((Obj*)class_name, CHARBUF)) {
        vtable = VTable_singleton(class_name, NULL);
    }
    else if (type_spec != NULL && Obj_Is_A((Obj*)type_spec, CHARBUF)) {
        if (CB_Equals_Str(type_spec, "i32_t", 5)) {
            vtable = INT32TYPE;
        }
        else if (CB_Equals_Str(type_spec, "i64_t", 5)) {
            vtable = INT64TYPE;
        }
        else if (CB_Equals_Str(type_spec, "f32_t", 5)) {
            vtable = FLOAT32TYPE;
        }
        else if (CB_Equals_Str(type_spec, "f64_t", 5)) {
            vtable = FLOAT64TYPE;
        }
        else {
            THROW(ERR, "Unrecognized type string: '%o'", type_spec);
        }
    }
    CERTIFY(vtable, VTABLE);
    NumericType *loaded = (NumericType*)VTable_Make_Obj(vtable);

    // Extract boost.
    Obj *boost_dump = Hash_Fetch_Str(source, "boost", 5);
    float boost = boost_dump ? (float)Obj_To_F64(boost_dump) : 1.0f;

    // Find boolean properties.
    Obj *indexed_dump = Hash_Fetch_Str(source, "indexed", 7);
    Obj *stored_dump  = Hash_Fetch_Str(source, "stored", 6);
    Obj *sort_dump    = Hash_Fetch_Str(source, "sortable", 8);
    bool_t indexed  = indexed_dump ? Obj_To_Bool(indexed_dump) : true;
    bool_t stored   = stored_dump  ? Obj_To_Bool(stored_dump)  : true;
    bool_t sortable = sort_dump    ? Obj_To_Bool(sort_dump)    : false;

    return NumType_init2(loaded, boost, indexed, stored, sortable);
}

/****************************************************************************/

Float64Type*
Float64Type_new() {
    Float64Type *self = (Float64Type*)VTable_Make_Obj(FLOAT64TYPE);
    return Float64Type_init(self);
}

Float64Type*
Float64Type_init(Float64Type *self) {
    return Float64Type_init2(self, 1.0, true, true, false);
}

Float64Type*
Float64Type_init2(Float64Type *self, float boost, bool_t indexed,
                  bool_t stored, bool_t sortable) {
    return (Float64Type*)NumType_init2((NumericType*)self, boost, indexed,
                                       stored, sortable);
}

CharBuf*
Float64Type_specifier(Float64Type *self) {
    UNUSED_VAR(self);
    return CB_newf("f64_t");
}

int8_t
Float64Type_primitive_id(Float64Type *self) {
    UNUSED_VAR(self);
    return FType_FLOAT64;
}

bool_t
Float64Type_equals(Float64Type *self, Obj *other) {
    if (self == (Float64Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, FLOAT64TYPE)) { return false; }
    Float64Type_equals_t super_equals = (Float64Type_equals_t)SUPER_METHOD(
                                            FLOAT64TYPE, Float64Type, Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Float32Type*
Float32Type_new() {
    Float32Type *self = (Float32Type*)VTable_Make_Obj(FLOAT32TYPE);
    return Float32Type_init(self);
}

Float32Type*
Float32Type_init(Float32Type *self) {
    return Float32Type_init2(self, 1.0, true, true, false);
}

Float32Type*
Float32Type_init2(Float32Type *self, float boost, bool_t indexed,
                  bool_t stored, bool_t sortable) {
    return (Float32Type*)NumType_init2((NumericType*)self, boost, indexed,
                                       stored, sortable);
}

CharBuf*
Float32Type_specifier(Float32Type *self) {
    UNUSED_VAR(self);
    return CB_newf("f32_t");
}

int8_t
Float32Type_primitive_id(Float32Type *self) {
    UNUSED_VAR(self);
    return FType_FLOAT32;
}

bool_t
Float32Type_equals(Float32Type *self, Obj *other) {
    if (self == (Float32Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, FLOAT32TYPE)) { return false; }
    Float32Type_equals_t super_equals = (Float32Type_equals_t)SUPER_METHOD(
                                            FLOAT32TYPE, Float32Type, Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Int32Type*
Int32Type_new() {
    Int32Type *self = (Int32Type*)VTable_Make_Obj(INT32TYPE);
    return Int32Type_init(self);
}

Int32Type*
Int32Type_init(Int32Type *self) {
    return Int32Type_init2(self, 1.0, true, true, false);
}

Int32Type*
Int32Type_init2(Int32Type *self, float boost, bool_t indexed,
                bool_t stored, bool_t sortable) {
    return (Int32Type*)NumType_init2((NumericType*)self, boost, indexed,
                                     stored, sortable);
}

CharBuf*
Int32Type_specifier(Int32Type *self) {
    UNUSED_VAR(self);
    return CB_newf("i32_t");
}

int8_t
Int32Type_primitive_id(Int32Type *self) {
    UNUSED_VAR(self);
    return FType_INT32;
}

bool_t
Int32Type_equals(Int32Type *self, Obj *other) {
    if (self == (Int32Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, INT32TYPE)) { return false; }
    Int32Type_equals_t super_equals = (Int32Type_equals_t)SUPER_METHOD(
                                          INT32TYPE, Int32Type, Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Int64Type*
Int64Type_new() {
    Int64Type *self = (Int64Type*)VTable_Make_Obj(INT64TYPE);
    return Int64Type_init(self);
}

Int64Type*
Int64Type_init(Int64Type *self) {
    return Int64Type_init2(self, 1.0, true, true, false);
}

Int64Type*
Int64Type_init2(Int64Type *self, float boost, bool_t indexed,
                bool_t stored, bool_t sortable) {
    return (Int64Type*)NumType_init2((NumericType*)self, boost, indexed,
                                     stored, sortable);
}

CharBuf*
Int64Type_specifier(Int64Type *self) {
    UNUSED_VAR(self);
    return CB_newf("i64_t");
}

int8_t
Int64Type_primitive_id(Int64Type *self) {
    UNUSED_VAR(self);
    return FType_INT64;
}

bool_t
Int64Type_equals(Int64Type *self, Obj *other) {
    if (self == (Int64Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, INT64TYPE)) { return false; }
    Int64Type_equals_t super_equals = (Int64Type_equals_t)SUPER_METHOD(
                                          INT64TYPE, Int64Type, Equals);
    return super_equals(self, other);
}


