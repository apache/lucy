#define C_LUCY_TESTOBJ
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Object/TestObj.h"

static Obj*
S_new_testobj()
{
    static ZombieCharBuf klass = ZCB_LITERAL("TestObj");
    Obj *obj;
    VTable *vtable = VTable_fetch_vtable((CharBuf*)&klass);
    if (!vtable) {
        vtable = VTable_singleton((CharBuf*)&klass, OBJ);
    }
    obj = VTable_Make_Obj(vtable);
    return Obj_init(obj);
}

static void
test_refcounts(TestBatch *batch)
{
    Obj     *obj      = S_new_testobj();

    ASSERT_INT_EQ(batch, Obj_Get_RefCount(obj), 1, 
        "Correct starting refcount");

    Obj_Inc_RefCount(obj);
    ASSERT_INT_EQ(batch, Obj_Get_RefCount(obj), 2, "Inc_RefCount" );

    Obj_Dec_RefCount(obj);
    ASSERT_INT_EQ(batch, Obj_Get_RefCount(obj), 1, "Dec_RefCount" );

    DECREF(obj);
}

static void
test_To_String(TestBatch *batch)
{
    Obj *testobj = S_new_testobj();
    CharBuf *string = Obj_To_String(testobj);
    ZombieCharBuf temp = ZCB_make(string);
    while(ZCB_Get_Size(&temp)) {
        if (ZCB_Starts_With_Str(&temp, "TestObj", 7)) { break; }
        ZCB_Nip_One(&temp);
    }
    ASSERT_TRUE(batch, ZCB_Starts_With_Str(&temp, "TestObj", 7), "To_String");
    DECREF(string);
    DECREF(testobj);
}

static void
test_Dump(TestBatch *batch)
{
    Obj *testobj = S_new_testobj();
    CharBuf *string = Obj_To_String(testobj);
    Obj *dump = Obj_Dump(testobj);
    ASSERT_TRUE(batch, Obj_Equals(dump, (Obj*)string), 
        "Default Dump returns To_String");
    DECREF(dump);
    DECREF(string);
    DECREF(testobj);
}

static void
test_Equals(TestBatch *batch)
{
    Obj *testobj = S_new_testobj();
    Obj *other   = S_new_testobj();

    ASSERT_TRUE(batch, Obj_Equals(testobj, testobj), 
        "Equals is true for the same object");
    ASSERT_FALSE(batch, Obj_Equals(testobj, other), 
        "Distinct objects are not equal");

    DECREF(testobj);
    DECREF(other);
}

static void
test_Hash_Code(TestBatch *batch)
{
    Obj *testobj = S_new_testobj();
    i32_t address = (i32_t)testobj;
    ASSERT_TRUE(batch, (Obj_Hash_Code(testobj) == address), 
        "Hash_Code uses memory address");
    DECREF(testobj);
}

static void
test_Is_A(TestBatch *batch)
{
    CharBuf *charbuf   = CB_new(0);
    VTable  *bb_vtable = CB_Get_VTable(charbuf);
    CharBuf *klass     = CB_Get_Class_Name(charbuf);

    ASSERT_TRUE(batch, CB_Is_A(charbuf, CHARBUF), "CharBuf Is_A CharBuf.");
    ASSERT_TRUE(batch, CB_Is_A(charbuf, OBJ), "CharBuf Is_A Obj.");
    ASSERT_TRUE(batch, bb_vtable == CHARBUF, "Get_VTable");
    ASSERT_TRUE(batch, CB_Equals(VTable_Get_Name(CHARBUF), (Obj*)klass),
        "Get_Class_Name");

    DECREF(charbuf);
}


void
TestObj_run_tests()
{
    TestBatch *batch = Test_new_batch("TestObj", 12, NULL);

    PLAN(batch);

    test_refcounts(batch);
    test_To_String(batch);
    test_Dump(batch);
    test_Equals(batch);
    test_Hash_Code(batch);
    test_Is_A(batch);

    batch->destroy(batch);
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

