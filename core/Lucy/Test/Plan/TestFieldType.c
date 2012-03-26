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

#define C_LUCY_TESTFIELDTYPE
#define C_LUCY_DUMMYFIELDTYPE
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Plan/TestFieldType.h"
#include "Lucy/Test/TestUtils.h"

DummyFieldType*
DummyFieldType_new() {
    DummyFieldType *self = (DummyFieldType*)VTable_Make_Obj(DUMMYFIELDTYPE);
    return (DummyFieldType*)FType_init((FieldType*)self);
}

static FieldType*
S_alt_field_type() {
    ZombieCharBuf *name = ZCB_WRAP_STR("DummyFieldType2", 15);
    VTable *vtable = VTable_singleton((CharBuf*)name, DUMMYFIELDTYPE);
    FieldType *self = (FieldType*)VTable_Make_Obj(vtable);
    return FType_init(self);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
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

    TEST_TRUE(batch, FType_Equals(type, (Obj*)other),
              "Equals() true with identical stats");
    TEST_FALSE(batch, FType_Equals(type, (Obj*)class_differs),
               "Equals() false with subclass");
    TEST_FALSE(batch, FType_Equals(type, (Obj*)class_differs),
               "Equals() false with super class");
    TEST_FALSE(batch, FType_Equals(type, (Obj*)boost_differs),
               "Equals() false with different boost");
    TEST_FALSE(batch, FType_Equals(type, (Obj*)indexed),
               "Equals() false with indexed => true");
    TEST_FALSE(batch, FType_Equals(type, (Obj*)stored),
               "Equals() false with stored => true");

    DECREF(stored);
    DECREF(indexed);
    DECREF(boost_differs);
    DECREF(other);
    DECREF(type);
}

static void
test_Compare_Values(TestBatch *batch) {
    FieldType     *type = (FieldType*)DummyFieldType_new();
    ZombieCharBuf *a    = ZCB_WRAP_STR("a", 1);
    ZombieCharBuf *b    = ZCB_WRAP_STR("b", 1);

    TEST_TRUE(batch,
              FType_Compare_Values(type, (Obj*)a, (Obj*)b) < 0,
              "a less than b");
    TEST_TRUE(batch,
              FType_Compare_Values(type, (Obj*)b, (Obj*)a) > 0,
              "b greater than a");
    TEST_TRUE(batch,
              FType_Compare_Values(type, (Obj*)b, (Obj*)b) == 0,
              "b equals b");

    DECREF(type);
}

void
TestFType_run_tests() {
    TestBatch *batch = TestBatch_new(9);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    test_Compare_Values(batch);
    DECREF(batch);
}


