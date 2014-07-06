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
NumType_init2(NumericType *self, float boost, bool indexed, bool stored,
              bool sortable) {
    FType_init((FieldType*)self);
    NumericTypeIVARS *const ivars = NumType_IVARS(self);
    ivars->boost      = boost;
    ivars->indexed    = indexed;
    ivars->stored     = stored;
    ivars->sortable   = sortable;
    return self;
}

bool
NumType_Binary_IMP(NumericType *self) {
    UNUSED_VAR(self);
    return true;
}

Hash*
NumType_Dump_For_Schema_IMP(NumericType *self) {
    NumericTypeIVARS *const ivars = NumType_IVARS(self);
    Hash *dump = Hash_new(0);
    Hash_Store_Utf8(dump, "type", 4, (Obj*)NumType_Specifier(self));

    // Store attributes that override the defaults.
    if (ivars->boost != 1.0) {
        Hash_Store_Utf8(dump, "boost", 5, (Obj*)Str_newf("%f64", ivars->boost));
    }
    if (!ivars->indexed) {
        Hash_Store_Utf8(dump, "indexed", 7, (Obj*)CFISH_FALSE);
    }
    if (!ivars->stored) {
        Hash_Store_Utf8(dump, "stored", 6, (Obj*)CFISH_FALSE);
    }
    if (ivars->sortable) {
        Hash_Store_Utf8(dump, "sortable", 8, (Obj*)CFISH_TRUE);
    }

    return dump;
}

Hash*
NumType_Dump_IMP(NumericType *self) {
    Hash *dump = NumType_Dump_For_Schema(self);
    Hash_Store_Utf8(dump, "_class", 6,
                    (Obj*)Str_Clone(NumType_Get_Class_Name(self)));
    DECREF(Hash_Delete_Utf8(dump, "type", 4));
    return dump;
}

NumericType*
NumType_Load_IMP(NumericType *self, Obj *dump) {
    UNUSED_VAR(self);
    Hash *source = (Hash*)CERTIFY(dump, HASH);

    // Get a Class
    String *class_name = (String*)Hash_Fetch_Utf8(source, "_class", 6);
    String *type_spec  = (String*)Hash_Fetch_Utf8(source, "type", 4);
    Class *klass = NULL;
    if (class_name != NULL && Obj_Is_A((Obj*)class_name, STRING)) {
        klass = Class_singleton(class_name, NULL);
    }
    else if (type_spec != NULL && Obj_Is_A((Obj*)type_spec, STRING)) {
        if (Str_Equals_Utf8(type_spec, "i32_t", 5)) {
            klass = INT32TYPE;
        }
        else if (Str_Equals_Utf8(type_spec, "i64_t", 5)) {
            klass = INT64TYPE;
        }
        else if (Str_Equals_Utf8(type_spec, "f32_t", 5)) {
            klass = FLOAT32TYPE;
        }
        else if (Str_Equals_Utf8(type_spec, "f64_t", 5)) {
            klass = FLOAT64TYPE;
        }
        else {
            THROW(ERR, "Unrecognized type string: '%o'", type_spec);
        }
    }
    CERTIFY(klass, CLASS);
    NumericType *loaded = (NumericType*)Class_Make_Obj(klass);

    // Extract boost.
    Obj *boost_dump = Hash_Fetch_Utf8(source, "boost", 5);
    float boost = boost_dump ? (float)Obj_To_F64(boost_dump) : 1.0f;

    // Find boolean properties.
    Obj *indexed_dump = Hash_Fetch_Utf8(source, "indexed", 7);
    Obj *stored_dump  = Hash_Fetch_Utf8(source, "stored", 6);
    Obj *sort_dump    = Hash_Fetch_Utf8(source, "sortable", 8);
    bool indexed  = indexed_dump ? Obj_To_Bool(indexed_dump) : true;
    bool stored   = stored_dump  ? Obj_To_Bool(stored_dump)  : true;
    bool sortable = sort_dump    ? Obj_To_Bool(sort_dump)    : false;

    return NumType_init2(loaded, boost, indexed, stored, sortable);
}

/****************************************************************************/

Float64Type*
Float64Type_new() {
    Float64Type *self = (Float64Type*)Class_Make_Obj(FLOAT64TYPE);
    return Float64Type_init(self);
}

Float64Type*
Float64Type_init(Float64Type *self) {
    return Float64Type_init2(self, 1.0, true, true, false);
}

Float64Type*
Float64Type_init2(Float64Type *self, float boost, bool indexed,
                  bool stored, bool sortable) {
    return (Float64Type*)NumType_init2((NumericType*)self, boost, indexed,
                                       stored, sortable);
}

String*
Float64Type_Specifier_IMP(Float64Type *self) {
    UNUSED_VAR(self);
    return Str_newf("f64_t");
}

int8_t
Float64Type_Primitive_ID_IMP(Float64Type *self) {
    UNUSED_VAR(self);
    return FType_FLOAT64;
}

bool
Float64Type_Equals_IMP(Float64Type *self, Obj *other) {
    if (self == (Float64Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, FLOAT64TYPE)) { return false; }
    Float64Type_Equals_t super_equals
        = SUPER_METHOD_PTR(FLOAT64TYPE, LUCY_Float64Type_Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Float32Type*
Float32Type_new() {
    Float32Type *self = (Float32Type*)Class_Make_Obj(FLOAT32TYPE);
    return Float32Type_init(self);
}

Float32Type*
Float32Type_init(Float32Type *self) {
    return Float32Type_init2(self, 1.0, true, true, false);
}

Float32Type*
Float32Type_init2(Float32Type *self, float boost, bool indexed,
                  bool stored, bool sortable) {
    return (Float32Type*)NumType_init2((NumericType*)self, boost, indexed,
                                       stored, sortable);
}

String*
Float32Type_Specifier_IMP(Float32Type *self) {
    UNUSED_VAR(self);
    return Str_newf("f32_t");
}

int8_t
Float32Type_Primitive_ID_IMP(Float32Type *self) {
    UNUSED_VAR(self);
    return FType_FLOAT32;
}

bool
Float32Type_Equals_IMP(Float32Type *self, Obj *other) {
    if (self == (Float32Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, FLOAT32TYPE)) { return false; }
    Float32Type_Equals_t super_equals
        = SUPER_METHOD_PTR(FLOAT32TYPE, LUCY_Float32Type_Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Int32Type*
Int32Type_new() {
    Int32Type *self = (Int32Type*)Class_Make_Obj(INT32TYPE);
    return Int32Type_init(self);
}

Int32Type*
Int32Type_init(Int32Type *self) {
    return Int32Type_init2(self, 1.0, true, true, false);
}

Int32Type*
Int32Type_init2(Int32Type *self, float boost, bool indexed,
                bool stored, bool sortable) {
    return (Int32Type*)NumType_init2((NumericType*)self, boost, indexed,
                                     stored, sortable);
}

String*
Int32Type_Specifier_IMP(Int32Type *self) {
    UNUSED_VAR(self);
    return Str_newf("i32_t");
}

int8_t
Int32Type_Primitive_ID_IMP(Int32Type *self) {
    UNUSED_VAR(self);
    return FType_INT32;
}

bool
Int32Type_Equals_IMP(Int32Type *self, Obj *other) {
    if (self == (Int32Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, INT32TYPE)) { return false; }
    Int32Type_Equals_t super_equals
        = SUPER_METHOD_PTR(INT32TYPE, LUCY_Int32Type_Equals);
    return super_equals(self, other);
}

/****************************************************************************/

Int64Type*
Int64Type_new() {
    Int64Type *self = (Int64Type*)Class_Make_Obj(INT64TYPE);
    return Int64Type_init(self);
}

Int64Type*
Int64Type_init(Int64Type *self) {
    return Int64Type_init2(self, 1.0, true, true, false);
}

Int64Type*
Int64Type_init2(Int64Type *self, float boost, bool indexed,
                bool stored, bool sortable) {
    return (Int64Type*)NumType_init2((NumericType*)self, boost, indexed,
                                     stored, sortable);
}

String*
Int64Type_Specifier_IMP(Int64Type *self) {
    UNUSED_VAR(self);
    return Str_newf("i64_t");
}

int8_t
Int64Type_Primitive_ID_IMP(Int64Type *self) {
    UNUSED_VAR(self);
    return FType_INT64;
}

bool
Int64Type_Equals_IMP(Int64Type *self, Obj *other) {
    if (self == (Int64Type*)other) { return true; }
    if (!other) { return false; }
    if (!Obj_Is_A(other, INT64TYPE)) { return false; }
    Int64Type_Equals_t super_equals
        = SUPER_METHOD_PTR(INT64TYPE, LUCY_Int64Type_Equals);
    return super_equals(self, other);
}


