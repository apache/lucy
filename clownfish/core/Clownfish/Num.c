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

#define C_LUCY_NUM
#define C_LUCY_INTNUM
#define C_LUCY_FLOATNUM
#define C_LUCY_INTEGER32
#define C_LUCY_INTEGER64
#define C_LUCY_FLOAT32
#define C_LUCY_FLOAT64
#define C_LUCY_BOOLNUM
#define C_LUCY_VIEWCHARBUF
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Num.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/VTable.h"

Num*
Num_init(Num *self) {
    ABSTRACT_CLASS_CHECK(self, NUM);
    return self;
}

bool_t
Num_equals(Num *self, Obj *other) {
    Num *twin = (Num*)other;
    if (twin == self) { return true; }
    if (!Obj_Is_A(other, NUM)) { return false; }
    if (Num_To_F64(self) != Num_To_F64(twin)) { return false; }
    if (Num_To_I64(self) != Num_To_I64(twin)) { return false; }
    return true;
}

/***************************************************************************/

FloatNum*
FloatNum_init(FloatNum *self) {
    ABSTRACT_CLASS_CHECK(self, FLOATNUM);
    return (FloatNum*)Num_init((Num*)self);
}

int32_t
FloatNum_compare_to(FloatNum *self, Obj *other) {
    Num *twin = (Num*)CERTIFY(other, NUM);
    double f64_diff = FloatNum_To_F64(self) - Num_To_F64(twin);
    if (f64_diff < 0)      { return -1; }
    else if (f64_diff > 0) { return 1;  }
    return 0;
}

CharBuf*
FloatNum_to_string(FloatNum *self) {
    return CB_newf("%f64", FloatNum_To_F64(self));
}

/***************************************************************************/

IntNum*
IntNum_init(IntNum *self) {
    ABSTRACT_CLASS_CHECK(self, INTNUM);
    return (IntNum*)Num_init((Num*)self);
}

int32_t
IntNum_compare_to(IntNum *self, Obj *other) {
    if (!Obj_Is_A(other, INTNUM)) {
        return -Obj_Compare_To(other, (Obj*)self);
    }
    int64_t self_value  = IntNum_To_I64(self);
    int64_t other_value = Obj_To_I64(other);
    if (self_value < other_value)      { return -1; }
    else if (self_value > other_value) { return 1;  }
    return 0;
}

CharBuf*
IntNum_to_string(IntNum *self) {
    return CB_newf("%i64", IntNum_To_I64(self));
}

/***************************************************************************/

Float32*
Float32_new(float value) {
    Float32 *self = (Float32*)VTable_Make_Obj(FLOAT32);
    return Float32_init(self, value);
}

Float32*
Float32_init(Float32 *self, float value) {
    self->value = value;
    return (Float32*)FloatNum_init((FloatNum*)self);
}

float
Float32_get_value(Float32 *self) {
    return self->value;
}

void
Float32_set_value(Float32 *self, float value) {
    self->value = value;
}

double
Float32_to_f64(Float32 *self) {
    return self->value;
}

int64_t
Float32_to_i64(Float32 *self) {
    return (int64_t)self->value;
}

int32_t
Float32_hash_sum(Float32 *self) {
    return *(int32_t*)&self->value;
}

Float32*
Float32_clone(Float32 *self) {
    return Float32_new(self->value);
}

void
Float32_mimic(Float32 *self, Obj *other) {
    Float32 *twin = (Float32*)CERTIFY(other, FLOAT32);
    self->value = twin->value;
}

/***************************************************************************/

Float64*
Float64_new(double value) {
    Float64 *self = (Float64*)VTable_Make_Obj(FLOAT64);
    return Float64_init(self, value);
}

Float64*
Float64_init(Float64 *self, double value) {
    self->value = value;
    return (Float64*)FloatNum_init((FloatNum*)self);
}

double
Float64_get_value(Float64 *self) {
    return self->value;
}

void
Float64_set_value(Float64 *self, double value) {
    self->value = value;
}

double
Float64_to_f64(Float64 *self) {
    return self->value;
}

int64_t
Float64_to_i64(Float64 *self) {
    return (int64_t)self->value;
}

Float64*
Float64_clone(Float64 *self) {
    return Float64_new(self->value);
}

void
Float64_mimic(Float64 *self, Obj *other) {
    Float64 *twin = (Float64*)CERTIFY(other, FLOAT64);
    self->value = twin->value;
}

int32_t
Float64_hash_sum(Float64 *self) {
    int32_t *ints = (int32_t*)&self->value;
    return ints[0] ^ ints[1];
}

/***************************************************************************/

Integer32*
Int32_new(int32_t value) {
    Integer32 *self = (Integer32*)VTable_Make_Obj(INTEGER32);
    return Int32_init(self, value);
}

Integer32*
Int32_init(Integer32 *self, int32_t value) {
    self->value = value;
    return (Integer32*)IntNum_init((IntNum*)self);
}

int32_t
Int32_get_value(Integer32 *self) {
    return self->value;
}

void
Int32_set_value(Integer32 *self, int32_t value) {
    self->value = value;
}

double
Int32_to_f64(Integer32 *self) {
    return self->value;
}

int64_t
Int32_to_i64(Integer32 *self) {
    return self->value;
}

Integer32*
Int32_clone(Integer32 *self) {
    return Int32_new(self->value);
}

void
Int32_mimic(Integer32 *self, Obj *other) {
    Integer32 *twin = (Integer32*)CERTIFY(other, INTEGER32);
    self->value = twin->value;
}

int32_t
Int32_hash_sum(Integer32 *self) {
    return self->value;
}

/***************************************************************************/

Integer64*
Int64_new(int64_t value) {
    Integer64 *self = (Integer64*)VTable_Make_Obj(INTEGER64);
    return Int64_init(self, value);
}

Integer64*
Int64_init(Integer64 *self, int64_t value) {
    self->value = value;
    return (Integer64*)IntNum_init((IntNum*)self);
}

int64_t
Int64_get_value(Integer64 *self) {
    return self->value;
}

void
Int64_set_value(Integer64 *self, int64_t value) {
    self->value = value;
}

double
Int64_to_f64(Integer64 *self) {
    return (double)self->value;
}

int64_t
Int64_to_i64(Integer64 *self) {
    return self->value;
}

Integer64*
Int64_clone(Integer64 *self) {
    return Int64_new(self->value);
}

void
Int64_mimic(Integer64 *self, Obj *other) {
    Integer64 *twin = (Integer64*)CERTIFY(other, INTEGER64);
    self->value = twin->value;
}

int32_t
Int64_hash_sum(Integer64 *self) {
    int32_t *ints = (int32_t*)&self->value;
    return ints[0] ^ ints[1];
}

bool_t
Int64_equals(Integer64 *self, Obj *other) {
    Num *twin = (Num*)other;
    if (twin == (Num*)self)         { return true; }
    if (!Obj_Is_A(other, NUM)) { return false; }
    if (Num_Is_A(twin, FLOATNUM)) {
        double  floating_val = Num_To_F64(twin);
        int64_t int_val      = (int64_t)floating_val;
        if ((double)int_val != floating_val) { return false; }
        if (int_val != self->value)          { return false; }
    }
    else {
        if (self->value != Num_To_I64(twin)) { return false; }
    }
    return true;
}

/***************************************************************************/


BoolNum *Bool_true_singleton;
BoolNum *Bool_false_singleton;

void
Bool_init_class() {
    Bool_true_singleton          = (BoolNum*)VTable_Make_Obj(BOOLNUM);
    Bool_true_singleton->value   = true;
    Bool_true_singleton->string  = CB_newf("true");
    Bool_false_singleton         = (BoolNum*)VTable_Make_Obj(BOOLNUM);
    Bool_false_singleton->value  = false;
    Bool_false_singleton->string = CB_newf("false");
}

BoolNum*
Bool_singleton(bool_t value) {
    return value ? CFISH_TRUE : CFISH_FALSE;
}

void
Bool_destroy(BoolNum *self) {
    if (self && self != CFISH_TRUE && self != CFISH_FALSE) {
        SUPER_DESTROY(self, BOOLNUM);
    }
}

bool_t
Bool_get_value(BoolNum *self) {
    return self->value;
}

double
Bool_to_f64(BoolNum *self) {
    return (double)self->value;
}

int64_t
Bool_to_i64(BoolNum *self) {
    return self->value;
}

bool_t
Bool_to_bool(BoolNum *self) {
    return self->value;
}

BoolNum*
Bool_clone(BoolNum *self) {
    return self;
}

int32_t
Bool_hash_sum(BoolNum *self) {
    int64_t hash_sum = PTR_TO_I64(self) + self->value;
    return (int32_t)hash_sum;
}

CharBuf*
Bool_to_string(BoolNum *self) {
    return (CharBuf*)CB_Inc_RefCount(self->string);
}

bool_t
Bool_equals(BoolNum *self, Obj *other) {
    return self == (BoolNum*)other;
}

BoolNum*
Bool_inc_refcount(BoolNum *self) {
    return self;
}

uint32_t
Bool_dec_refcount(BoolNum *self) {
    return 1;
}

