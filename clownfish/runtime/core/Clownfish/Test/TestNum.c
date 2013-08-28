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

#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "Clownfish/Test/TestNum.h"

#include "Clownfish/String.h"
#include "Clownfish/Num.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Clownfish/VTable.h"

TestNum*
TestNum_new() {
    return (TestNum*)VTable_Make_Obj(TESTNUM);
}

static void
test_To_String(TestBatchRunner *runner) {
    Float32   *f32 = Float32_new(1.33f);
    Float64   *f64 = Float64_new(1.33);
    Integer32 *i32 = Int32_new(INT32_MAX);
    Integer64 *i64 = Int64_new(INT64_MAX);
    String *f32_string = Float32_To_String(f32);
    String *f64_string = Float64_To_String(f64);
    String *i32_string = Int32_To_String(i32);
    String *i64_string = Int64_To_String(i64);
    String *true_string  = Bool_To_String(CFISH_TRUE);
    String *false_string = Bool_To_String(CFISH_FALSE);

    TEST_TRUE(runner, Str_Starts_With_Str(f32_string, "1.3", 3),
              "Float32_To_String");
    TEST_TRUE(runner, Str_Starts_With_Str(f64_string, "1.3", 3),
              "Float64_To_String");
    TEST_TRUE(runner, Str_Equals_Str(i32_string, "2147483647", 10),
              "Int32_To_String");
    TEST_TRUE(runner, Str_Equals_Str(i64_string, "9223372036854775807", 19),
              "Int64_To_String");
    TEST_TRUE(runner, Str_Equals_Str(true_string, "true", 4),
              "Bool_To_String [true]");
    TEST_TRUE(runner, Str_Equals_Str(false_string, "false", 5),
              "Bool_To_String [false]");

    DECREF(false_string);
    DECREF(true_string);
    DECREF(i64_string);
    DECREF(i32_string);
    DECREF(f64_string);
    DECREF(f32_string);
    DECREF(i64);
    DECREF(i32);
    DECREF(f64);
    DECREF(f32);
}

static void
test_accessors(TestBatchRunner *runner) {
    Float32   *f32 = Float32_new(1.0);
    Float64   *f64 = Float64_new(1.0);
    Integer32 *i32 = Int32_new(1);
    Integer64 *i64 = Int64_new(1);
    float  wanted32 = 1.33f;
    double wanted64 = 1.33;
    float  got32;
    double got64;

    Float32_Set_Value(f32, 1.33f);
    TEST_FLOAT_EQ(runner, Float32_Get_Value(f32), 1.33f,
                  "F32 Set_Value Get_Value");

    Float64_Set_Value(f64, 1.33);
    got64 = Float64_Get_Value(f64);
    TEST_TRUE(runner, *(int64_t*)&got64 == *(int64_t*)&wanted64,
              "F64 Set_Value Get_Value");

    TEST_TRUE(runner, Float32_To_I64(f32) == 1, "Float32_To_I64");
    TEST_TRUE(runner, Float64_To_I64(f64) == 1, "Float64_To_I64");

    got32 = (float)Float32_To_F64(f32);
    TEST_TRUE(runner, *(int32_t*)&got32 == *(int32_t*)&wanted32,
              "Float32_To_F64");

    got64 = Float64_To_F64(f64);
    TEST_TRUE(runner, *(int64_t*)&got64 == *(int64_t*)&wanted64,
              "Float64_To_F64");

    Int32_Set_Value(i32, INT32_MIN);
    TEST_INT_EQ(runner, Int32_Get_Value(i32), INT32_MIN,
                "I32 Set_Value Get_Value");

    Int64_Set_Value(i64, INT64_MIN);
    TEST_TRUE(runner, Int64_Get_Value(i64) == INT64_MIN,
              "I64 Set_Value Get_Value");

    Int32_Set_Value(i32, -1);
    Int64_Set_Value(i64, -1);
    TEST_TRUE(runner, Int32_To_F64(i32) == -1, "Int32_To_F64");
    TEST_TRUE(runner, Int64_To_F64(i64) == -1, "Int64_To_F64");

    TEST_INT_EQ(runner, Bool_Get_Value(CFISH_TRUE), true,
                "Bool_Get_Value [true]");
    TEST_INT_EQ(runner, Bool_Get_Value(CFISH_FALSE), false,
                "Bool_Get_Value [false]");
    TEST_TRUE(runner, Bool_To_I64(CFISH_TRUE) == true,
              "Bool_To_I64 [true]");
    TEST_TRUE(runner, Bool_To_I64(CFISH_FALSE) == false,
              "Bool_To_I64 [false]");
    TEST_TRUE(runner, Bool_To_F64(CFISH_TRUE) == 1.0,
              "Bool_To_F64 [true]");
    TEST_TRUE(runner, Bool_To_F64(CFISH_FALSE) == 0.0,
              "Bool_To_F64 [false]");

    DECREF(i64);
    DECREF(i32);
    DECREF(f64);
    DECREF(f32);
}

static void
test_Equals_and_Compare_To(TestBatchRunner *runner) {
    Float32   *f32 = Float32_new(1.0);
    Float64   *f64 = Float64_new(1.0);
    Integer32 *i32 = Int32_new(INT32_MAX);
    Integer64 *i64 = Int64_new(INT64_MAX);

    TEST_TRUE(runner, Float32_Compare_To(f32, (Obj*)f64) == 0,
              "F32_Compare_To equal");
    TEST_TRUE(runner, Float32_Equals(f32, (Obj*)f64),
              "F32_Equals equal");

    Float64_Set_Value(f64, 2.0);
    TEST_TRUE(runner, Float32_Compare_To(f32, (Obj*)f64) < 0,
              "F32_Compare_To less than");
    TEST_FALSE(runner, Float32_Equals(f32, (Obj*)f64),
               "F32_Equals less than");

    Float64_Set_Value(f64, 0.0);
    TEST_TRUE(runner, Float32_Compare_To(f32, (Obj*)f64) > 0,
              "F32_Compare_To greater than");
    TEST_FALSE(runner, Float32_Equals(f32, (Obj*)f64),
               "F32_Equals greater than");

    Float64_Set_Value(f64, 1.0);
    Float32_Set_Value(f32, 1.0);
    TEST_TRUE(runner, Float64_Compare_To(f64, (Obj*)f32) == 0,
              "F64_Compare_To equal");
    TEST_TRUE(runner, Float64_Equals(f64, (Obj*)f32),
              "F64_Equals equal");

    Float32_Set_Value(f32, 2.0);
    TEST_TRUE(runner, Float64_Compare_To(f64, (Obj*)f32) < 0,
              "F64_Compare_To less than");
    TEST_FALSE(runner, Float64_Equals(f64, (Obj*)f32),
               "F64_Equals less than");

    Float32_Set_Value(f32, 0.0);
    TEST_TRUE(runner, Float64_Compare_To(f64, (Obj*)f32) > 0,
              "F64_Compare_To greater than");
    TEST_FALSE(runner, Float64_Equals(f64, (Obj*)f32),
               "F64_Equals greater than");

    Float64_Set_Value(f64, INT64_MAX * 2.0);
    TEST_TRUE(runner, Float64_Compare_To(f64, (Obj*)i64) > 0,
              "Float64 comparison to Integer64");
    TEST_TRUE(runner, Int64_Compare_To(i64, (Obj*)f64) < 0,
              "Integer64 comparison to Float64");

    Float32_Set_Value(f32, INT32_MAX * 2.0f);
    TEST_TRUE(runner, Float32_Compare_To(f32, (Obj*)i32) > 0,
              "Float32 comparison to Integer32");
    TEST_TRUE(runner, Int32_Compare_To(i32, (Obj*)f32) < 0,
              "Integer32 comparison to Float32");

    Int64_Set_Value(i64, INT64_C(0x6666666666666666));
    Integer64 *i64_copy = Int64_new(INT64_C(0x6666666666666666));
    TEST_TRUE(runner, Int64_Compare_To(i64, (Obj*)i64_copy) == 0,
              "Integer64 comparison to same number");

    TEST_TRUE(runner, Bool_Equals(CFISH_TRUE, (Obj*)CFISH_TRUE),
              "CFISH_TRUE Equals itself");
    TEST_TRUE(runner, Bool_Equals(CFISH_FALSE, (Obj*)CFISH_FALSE),
              "CFISH_FALSE Equals itself");
    TEST_FALSE(runner, Bool_Equals(CFISH_FALSE, (Obj*)CFISH_TRUE),
               "CFISH_FALSE not Equals CFISH_TRUE ");
    TEST_FALSE(runner, Bool_Equals(CFISH_TRUE, (Obj*)CFISH_FALSE),
               "CFISH_TRUE not Equals CFISH_FALSE ");
    TEST_FALSE(runner, Bool_Equals(CFISH_TRUE, (Obj*)STRING),
               "CFISH_TRUE not Equals random other object ");

    DECREF(i64_copy);
    DECREF(i64);
    DECREF(i32);
    DECREF(f64);
    DECREF(f32);
}

static void
test_Clone(TestBatchRunner *runner) {
    Float32   *f32 = Float32_new(1.33f);
    Float64   *f64 = Float64_new(1.33);
    Integer32 *i32 = Int32_new(INT32_MAX);
    Integer64 *i64 = Int64_new(INT64_MAX);
    Float32   *f32_dupe = Float32_Clone(f32);
    Float64   *f64_dupe = Float64_Clone(f64);
    Integer32 *i32_dupe = Int32_Clone(i32);
    Integer64 *i64_dupe = Int64_Clone(i64);
    TEST_TRUE(runner, Float32_Equals(f32, (Obj*)f32_dupe),
              "Float32 Clone");
    TEST_TRUE(runner, Float64_Equals(f64, (Obj*)f64_dupe),
              "Float64 Clone");
    TEST_TRUE(runner, Int32_Equals(i32, (Obj*)i32_dupe),
              "Integer32 Clone");
    TEST_TRUE(runner, Int64_Equals(i64, (Obj*)i64_dupe),
              "Integer64 Clone");
    TEST_TRUE(runner, Bool_Equals(CFISH_TRUE, (Obj*)Bool_Clone(CFISH_TRUE)),
              "BoolNum Clone");
    DECREF(i64_dupe);
    DECREF(i32_dupe);
    DECREF(f64_dupe);
    DECREF(f32_dupe);
    DECREF(i64);
    DECREF(i32);
    DECREF(f64);
    DECREF(f32);
}

static void
test_Mimic(TestBatchRunner *runner) {
    Float32   *f32 = Float32_new(1.33f);
    Float64   *f64 = Float64_new(1.33);
    Integer32 *i32 = Int32_new(INT32_MAX);
    Integer64 *i64 = Int64_new(INT64_MAX);
    Float32   *f32_dupe = Float32_new(0.0f);
    Float64   *f64_dupe = Float64_new(0.0);
    Integer32 *i32_dupe = Int32_new(0);
    Integer64 *i64_dupe = Int64_new(0);
    Float32_Mimic(f32_dupe, (Obj*)f32);
    Float64_Mimic(f64_dupe, (Obj*)f64);
    Int32_Mimic(i32_dupe, (Obj*)i32);
    Int64_Mimic(i64_dupe, (Obj*)i64);
    TEST_TRUE(runner, Float32_Equals(f32, (Obj*)f32_dupe),
              "Float32 Mimic");
    TEST_TRUE(runner, Float64_Equals(f64, (Obj*)f64_dupe),
              "Float64 Mimic");
    TEST_TRUE(runner, Int32_Equals(i32, (Obj*)i32_dupe),
              "Integer32 Mimic");
    TEST_TRUE(runner, Int64_Equals(i64, (Obj*)i64_dupe),
              "Integer64 Mimic");
    DECREF(i64_dupe);
    DECREF(i32_dupe);
    DECREF(f64_dupe);
    DECREF(f32_dupe);
    DECREF(i64);
    DECREF(i32);
    DECREF(f64);
    DECREF(f32);
}

void
TestNum_Run_IMP(TestNum *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 53);
    test_To_String(runner);
    test_accessors(runner);
    test_Equals_and_Compare_To(runner);
    test_Clone(runner);
    test_Mimic(runner);
}


