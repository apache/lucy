#define C_LUCY_NUM
#define C_LUCY_INTNUM
#define C_LUCY_FLOATNUM
#define C_LUCY_INTEGER32
#define C_LUCY_INTEGER64
#define C_LUCY_FLOAT32
#define C_LUCY_FLOAT64
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Lucy/Object/Num.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/VTable.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

Num*
Num_init(Num *self)
{
    ABSTRACT_CLASS_CHECK(self, NUM);
    return self;
}

bool_t
Num_equals(Num *self, Obj *other)
{
    Num *evil_twin = (Num*)other;
    if (evil_twin == self) { return true; }
    if (!Obj_Is_A(evil_twin, NUM)) { return false; }
    if (Num_To_F64(self) != Num_To_F64(evil_twin)) { return false; }
    if (Num_To_I64(self) != Num_To_I64(evil_twin)) { return false; }
    return true;
}

i32_t
Num_compare_to(Num *self, Obj *other)
{
    Num *evil_twin = (Num*)CERTIFY(other, NUM);
    double f64_diff = Num_To_F64(self) - Num_To_F64(evil_twin);
    if (f64_diff) {
        if      (f64_diff < 0) { return -1; }
        else if (f64_diff > 0) { return 1;  }
    }
    else {
        i64_t i64_diff = Num_To_I64(self) - Num_To_I64(evil_twin);
        if (i64_diff) { 
            if      (i64_diff < 0) { return -1; }
            else if (i64_diff > 0) { return 1;  }
        }
    }
    return 0;
}

/***************************************************************************/

FloatNum*
FloatNum_init(FloatNum *self)
{
    ABSTRACT_CLASS_CHECK(self, FLOATNUM);
    return (FloatNum*)Num_init((Num*)self);
}

CharBuf*
FloatNum_to_string(FloatNum *self)
{
    return CB_newf("%f64", FloatNum_To_F64(self));
}

/***************************************************************************/

IntNum*
IntNum_init(IntNum *self)
{
    ABSTRACT_CLASS_CHECK(self, INTNUM);
    return (IntNum*)Num_init((Num*)self);
}

CharBuf*
IntNum_to_string(IntNum *self)
{
    return CB_newf("%i64", IntNum_To_I64(self));
}

/***************************************************************************/

Float32*
Float32_new(float value)
{
    Float32 *self = (Float32*)VTable_Make_Obj(FLOAT32);
    return Float32_init(self, value);
}

Float32*
Float32_init(Float32 *self, float value)
{
    self->value = value;
    return (Float32*)FloatNum_init((FloatNum*)self);
}

float
Float32_get_value(Float32 *self) { return self->value; }
void
Float32_set_value(Float32 *self, float value) { self->value = value; }

double
Float32_to_f64(Float32 *self)
{
    return self->value;
}

i64_t
Float32_to_i64(Float32 *self)
{
    return (i64_t)self->value;
}

i32_t
Float32_hash_code(Float32 *self)
{
    return *(i32_t*)&self->value;
}

Float32*
Float32_clone(Float32 *self)
{
    return Float32_new(self->value);
}

void
Float32_serialize(Float32 *self, OutStream *outstream)
{
    OutStream_Write_F32(outstream, self->value);
}

Float32*
Float32_deserialize(Float32 *self, InStream *instream)
{
    float value = InStream_Read_F32(instream);
    return self ? Float32_init(self, value) : Float32_new(value);
}

/***************************************************************************/

Float64*
Float64_new(double value)
{
    Float64 *self = (Float64*)VTable_Make_Obj(FLOAT64);
    return Float64_init(self, value);
}

Float64*
Float64_init(Float64 *self, double value)
{
    self->value = value;
    return (Float64*)FloatNum_init((FloatNum*)self);
}

double
Float64_get_value(Float64 *self) { return self->value; }
void
Float64_set_value(Float64 *self, double value) { self->value = value; }

double
Float64_to_f64(Float64 *self)
{
    return self->value;
}

i64_t
Float64_to_i64(Float64 *self)
{
    return (i64_t)self->value;
}

Float64*
Float64_clone(Float64 *self)
{
    return Float64_new(self->value);
}

i32_t
Float64_hash_code(Float64 *self)
{
    i32_t *ints = (i32_t*)&self->value;
    return ints[0] ^ ints[1];
}

void
Float64_serialize(Float64 *self, OutStream *outstream)
{
    OutStream_Write_F64(outstream, self->value);
}

Float64*
Float64_deserialize(Float64 *self, InStream *instream)
{
    double value = InStream_Read_F64(instream);
    return self ? Float64_init(self, value) : Float64_new(value);
}

/***************************************************************************/

Integer32*
Int32_new(i32_t value)
{
    Integer32 *self = (Integer32*)VTable_Make_Obj(INTEGER32);
    return Int32_init(self, value);
}

Integer32*
Int32_init(Integer32 *self, i32_t value)
{
    self->value = value;
    return (Integer32*)IntNum_init((IntNum*)self);
}

i32_t
Int32_get_value(Integer32 *self) { return self->value; }
void
Int32_set_value(Integer32 *self, i32_t value) { self->value = value; }

double
Int32_to_f64(Integer32 *self)
{
    return self->value;
}

i64_t
Int32_to_i64(Integer32 *self)
{
    return self->value;
}

Integer32*
Int32_clone(Integer32 *self)
{
    return Int32_new(self->value);
}

i32_t
Int32_hash_code(Integer32 *self)
{
    return self->value;
}

void
Int32_serialize(Integer32 *self, OutStream *outstream)
{
    OutStream_Write_C32(outstream, (u32_t)self->value);
}

Integer32*
Int32_deserialize(Integer32 *self, InStream *instream)
{
    i32_t value = (i32_t)InStream_Read_C32(instream);
    return self ? Int32_init(self, value) : Int32_new(value);
}

/***************************************************************************/

Integer64*
Int64_new(i64_t value)
{
    Integer64 *self = (Integer64*)VTable_Make_Obj(INTEGER64);
    return Int64_init(self, value);
}

Integer64*
Int64_init(Integer64 *self, i64_t value)
{
    self->value = value;
    return (Integer64*)FloatNum_init((FloatNum*)self);
}

i64_t
Int64_get_value(Integer64 *self) { return self->value; }
void
Int64_set_value(Integer64 *self, i64_t value) { self->value = value; }

double
Int64_to_f64(Integer64 *self)
{
    return (double)self->value;
}

i64_t
Int64_to_i64(Integer64 *self)
{
    return self->value;
}

Integer64*
Int64_clone(Integer64 *self)
{
    return Int64_new(self->value);
}

i32_t
Int64_hash_code(Integer64 *self)
{
    i32_t *ints = (i32_t*)&self->value;
    return ints[0] ^ ints[1];
}

bool_t
Int64_equals(Integer64 *self, Obj *other)
{
    Num *evil_twin = (Num*)other;
    if (evil_twin == (Num*)self)         { return true; }
    if (!Obj_Is_A(evil_twin, NUM)) { return false; }
    if (Obj_Is_A(evil_twin, FLOATNUM)) {
        double floating_val = Num_To_F64(evil_twin);
        i64_t  int_val      = (i64_t)floating_val;
        if ((double)int_val != floating_val) { return false; }
        if (int_val != self->value)          { return false; }
    }
    else {
        if (self->value != Num_To_I64(evil_twin)) { return false; }
    }
    return true;
}

void
Int64_serialize(Integer64 *self, OutStream *outstream)
{
    OutStream_Write_C64(outstream, (u64_t)self->value);
}

Integer64*
Int64_deserialize(Integer64 *self, InStream *instream)
{
    i64_t value = (i64_t)InStream_Read_C64(instream);
    return self ? Int64_init(self, value) : Int64_new(value);
}

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

