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

#define C_TESTLUCY_TESTFIELDTYPE
#define C_LUCY_DUMMYFIELDTYPE
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/TestUtils.h"

TestFieldType*
TestFType_new() {
    return (TestFieldType*)Class_Make_Obj(TESTFIELDTYPE);
}

DummyFieldType*
DummyFieldType_new() {
    DummyFieldType *self = (DummyFieldType*)Class_Make_Obj(DUMMYFIELDTYPE);
    return (DummyFieldType*)FType_init((FieldType*)self);
}

static FieldType*
S_alt_field_type() {
    StackString *name = SSTR_WRAP_UTF8("DummyFieldType2", 15);
    Class *klass = Class_singleton((String*)name, DUMMYFIELDTYPE);
    FieldType *self = (FieldType*)Class_Make_Obj(klass);
    return FType_init(self);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    FieldType   *type          = (FieldType*)DummyFieldType_new();
    FieldType   *other         = (FieldType*)DummyFieldType_new();
    FieldType   *class_differs = S_alt_field_type();
    FieldType   *boost_differs = (FieldType*)DummyFieldType_new();
    FieldType   *indexed       = (FieldType*)DummyFieldType_new();
    FieldType   *stored        = (FieldType*)DummyFieldType_new();

    FType_Set_Boost(other, 1.0);
    FType_Set_Indexed(indexed, false);
    FType_Set_Stored(stored, false);

    FType_Set_Boost(boost_differs, 1.5);
    FType_Set_Indexed(indexed, true);
    FType_Set_Stored(stored, true);

    TEST_TRUE(runner, FType_Equals(type, (Obj*)other),
              "Equals() true with identical stats");
    TEST_FALSE(runner, FType_Equals(type, (Obj*)class_differs),
               "Equals() false with subclass");
    TEST_FALSE(runner, FType_Equals(type, (Obj*)class_differs),
               "Equals() false with super class");
    TEST_FALSE(runner, FType_Equals(type, (Obj*)boost_differs),
               "Equals() false with different boost");
    TEST_FALSE(runner, FType_Equals(type, (Obj*)indexed),
               "Equals() false with indexed => true");
    TEST_FALSE(runner, FType_Equals(type, (Obj*)stored),
               "Equals() false with stored => true");

    DECREF(stored);
    DECREF(indexed);
    DECREF(boost_differs);
    DECREF(class_differs);
    DECREF(other);
    DECREF(type);
}

static void
test_Compare_Values(TestBatchRunner *runner) {
    FieldType     *type = (FieldType*)DummyFieldType_new();
    StackString *a    = SSTR_WRAP_UTF8("a", 1);
    StackString *b    = SSTR_WRAP_UTF8("b", 1);

    TEST_TRUE(runner,
              FType_Compare_Values(type, (Obj*)a, (Obj*)b) < 0,
              "a less than b");
    TEST_TRUE(runner,
              FType_Compare_Values(type, (Obj*)b, (Obj*)a) > 0,
              "b greater than a");
    TEST_TRUE(runner,
              FType_Compare_Values(type, (Obj*)b, (Obj*)b) == 0,
              "b equals b");

    DECREF(type);
}

void
TestFType_Run_IMP(TestFieldType *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 9);
    test_Dump_Load_and_Equals(runner);
    test_Compare_Values(runner);
}


