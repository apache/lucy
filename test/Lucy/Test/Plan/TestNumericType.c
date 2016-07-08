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

#define C_TESTLUCY_TESTNUMERICTYPE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestNumericType.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Plan/BlobType.h"
#include "Lucy/Plan/NumericType.h"
#include "Lucy/Util/Freezer.h"

TestNumericType*
TestNumericType_new() {
    return (TestNumericType*)Class_Make_Obj(TESTNUMERICTYPE);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    Int32Type   *i32 = Int32Type_new();
    Int64Type   *i64 = Int64Type_new();
    Float32Type *f32 = Float32Type_new();
    Float64Type *f64 = Float64Type_new();

    TEST_FALSE(runner, Int32Type_Equals(i32, (Obj*)i64),
               "Int32Type_Equals() false for different type");
    TEST_FALSE(runner, Int32Type_Equals(i32, NULL),
               "Int32Type_Equals() false for NULL");

    TEST_FALSE(runner, Int64Type_Equals(i64, (Obj*)i32),
               "Int64Type_Equals() false for different type");
    TEST_FALSE(runner, Int64Type_Equals(i64, NULL),
               "Int64Type_Equals() false for NULL");

    TEST_FALSE(runner, Float32Type_Equals(f32, (Obj*)f64),
               "Float32Type_Equals() false for different type");
    TEST_FALSE(runner, Float32Type_Equals(f32, NULL),
               "Float32Type_Equals() false for NULL");

    TEST_FALSE(runner, Float64Type_Equals(f64, (Obj*)f32),
               "Float64Type_Equals() false for different type");
    TEST_FALSE(runner, Float64Type_Equals(f64, NULL),
               "Float64Type_Equals() false for NULL");

    {
        Obj *dump = (Obj*)Int32Type_Dump(i32);
        Obj *other = Freezer_load(dump);
        TEST_TRUE(runner, Int32Type_Equals(i32, other),
                  "Dump => Load round trip for Int32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Int64Type_Dump(i64);
        Obj *other = Freezer_load(dump);
        TEST_TRUE(runner, Int64Type_Equals(i64, other),
                  "Dump => Load round trip for Int64Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float32Type_Dump(f32);
        Obj *other = Freezer_load(dump);
        TEST_TRUE(runner, Float32Type_Equals(f32, other),
                  "Dump => Load round trip for Float32Type");
        DECREF(dump);
        DECREF(other);
    }

    {
        Obj *dump = (Obj*)Float64Type_Dump(f64);
        Obj *other = Freezer_load(dump);
        TEST_TRUE(runner, Float64Type_Equals(f64, other),
                  "Dump => Load round trip for Float64Type");
        DECREF(dump);
        DECREF(other);
    }

    DECREF(i32);
    DECREF(i64);
    DECREF(f32);
    DECREF(f64);
}

void
TestNumericType_Run_IMP(TestNumericType *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 12);
    test_Dump_Load_and_Equals(runner);
}


