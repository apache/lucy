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

#include <string.h>

#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/Test/TestVArray.h"

#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/Num.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

TestVArray*
TestVArray_new() {
    return (TestVArray*)VTable_Make_Obj(TESTVARRAY);
}

static void
test_Equals(TestBatchRunner *runner) {
    VArray *array = VA_new(0);
    VArray *other = VA_new(0);
    StackString *stuff = SSTR_WRAP_STR("stuff", 5);

    TEST_TRUE(runner, VA_Equals(array, (Obj*)other),
              "Empty arrays are equal");

    VA_Push(array, (Obj*)CFISH_TRUE);
    TEST_FALSE(runner, VA_Equals(array, (Obj*)other),
               "Add one elem and Equals returns false");

    VA_Push(other, (Obj*)CFISH_TRUE);
    TEST_TRUE(runner, VA_Equals(array, (Obj*)other),
              "Add a matching elem and Equals returns true");

    VA_Store(array, 2, (Obj*)CFISH_TRUE);
    TEST_FALSE(runner, VA_Equals(array, (Obj*)other),
               "Add elem after a NULL and Equals returns false");

    VA_Store(other, 2, (Obj*)CFISH_TRUE);
    TEST_TRUE(runner, VA_Equals(array, (Obj*)other),
              "Empty elems don't spoil Equals");

    VA_Store(other, 2, INCREF(stuff));
    TEST_FALSE(runner, VA_Equals(array, (Obj*)other),
               "Non-matching value spoils Equals");

    VA_Excise(array, 1, 2); // removes empty elems
    VA_Delete(other, 1);    // leaves NULL in place of deleted elem
    VA_Delete(other, 2);
    TEST_FALSE(runner, VA_Equals(array, (Obj*)other),
               "Empty trailing elements spoil Equals");

    DECREF(array);
    DECREF(other);
}

static void
test_Store_Fetch(TestBatchRunner *runner) {
    VArray *array = VA_new(0);
    String *elem;

    TEST_TRUE(runner, VA_Fetch(array, 2) == NULL, "Fetch beyond end");

    VA_Store(array, 2, (Obj*)Str_newf("foo"));
    elem = (String*)CERTIFY(VA_Fetch(array, 2), STRING);
    TEST_INT_EQ(runner, 3, VA_Get_Size(array), "Store updates size");
    TEST_TRUE(runner, Str_Equals_Str(elem, "foo", 3), "Store");

    INCREF(elem);
    TEST_INT_EQ(runner, 2, Str_Get_RefCount(elem),
                "start with refcount of 2");
    VA_Store(array, 2, (Obj*)Str_newf("bar"));
    TEST_INT_EQ(runner, 1, Str_Get_RefCount(elem),
                "Displacing elem via Store updates refcount");
    DECREF(elem);
    elem = (String*)CERTIFY(VA_Fetch(array, 2), STRING);
    TEST_TRUE(runner, Str_Equals_Str(elem, "bar", 3), "Store displacement");

    DECREF(array);
}

static void
test_Push_Pop_Shift_Unshift(TestBatchRunner *runner) {
    VArray *array = VA_new(0);
    String *elem;

    TEST_INT_EQ(runner, VA_Get_Size(array), 0, "size starts at 0");
    VA_Push(array, (Obj*)Str_newf("a"));
    VA_Push(array, (Obj*)Str_newf("b"));
    VA_Push(array, (Obj*)Str_newf("c"));

    TEST_INT_EQ(runner, VA_Get_Size(array), 3, "size after Push");
    TEST_TRUE(runner, NULL != CERTIFY(VA_Fetch(array, 2), STRING), "Push");

    elem = (String*)CERTIFY(VA_Shift(array), STRING);
    TEST_TRUE(runner, Str_Equals_Str(elem, "a", 1), "Shift");
    TEST_INT_EQ(runner, VA_Get_Size(array), 2, "size after Shift");
    DECREF(elem);

    elem = (String*)CERTIFY(VA_Pop(array), STRING);
    TEST_TRUE(runner, Str_Equals_Str(elem, "c", 1), "Pop");
    TEST_INT_EQ(runner, VA_Get_Size(array), 1, "size after Pop");
    DECREF(elem);

    VA_Unshift(array, (Obj*)Str_newf("foo"));
    elem = (String*)CERTIFY(VA_Fetch(array, 0), STRING);
    TEST_TRUE(runner, Str_Equals_Str(elem, "foo", 3), "Unshift");
    TEST_INT_EQ(runner, VA_Get_Size(array), 2, "size after Shift");

    DECREF(array);
}

static void
test_Delete(TestBatchRunner *runner) {
    VArray *wanted = VA_new(5);
    VArray *got    = VA_new(5);
    uint32_t i;

    for (i = 0; i < 5; i++) { VA_Push(got, (Obj*)Str_newf("%u32", i)); }
    VA_Store(wanted, 0, (Obj*)Str_newf("0", i));
    VA_Store(wanted, 1, (Obj*)Str_newf("1", i));
    VA_Store(wanted, 4, (Obj*)Str_newf("4", i));
    DECREF(VA_Delete(got, 2));
    DECREF(VA_Delete(got, 3));
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got), "Delete");

    DECREF(wanted);
    DECREF(got);
}

static void
test_Resize(TestBatchRunner *runner) {
    VArray *array = VA_new(3);
    uint32_t i;

    for (i = 0; i < 2; i++) { VA_Push(array, (Obj*)Str_newf("%u32", i)); }
    TEST_INT_EQ(runner, VA_Get_Capacity(array), 3, "Start with capacity 3");

    VA_Resize(array, 4);
    TEST_INT_EQ(runner, VA_Get_Size(array), 4, "Resize up");
    TEST_INT_EQ(runner, VA_Get_Capacity(array), 4,
                "Resize changes capacity");

    VA_Resize(array, 2);
    TEST_INT_EQ(runner, VA_Get_Size(array), 2, "Resize down");
    TEST_TRUE(runner, VA_Fetch(array, 2) == NULL, "Resize down zaps elem");

    DECREF(array);
}

static void
test_Excise(TestBatchRunner *runner) {
    VArray *wanted = VA_new(5);
    VArray *got    = VA_new(5);

    for (uint32_t i = 0; i < 5; i++) {
        VA_Push(wanted, (Obj*)Str_newf("%u32", i));
        VA_Push(got, (Obj*)Str_newf("%u32", i));
    }

    VA_Excise(got, 7, 1);
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got),
              "Excise outside of range is no-op");

    VA_Excise(got, 2, 2);
    DECREF(VA_Delete(wanted, 2));
    DECREF(VA_Delete(wanted, 3));
    VA_Store(wanted, 2, VA_Delete(wanted, 4));
    VA_Resize(wanted, 3);
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got),
              "Excise multiple elems");

    VA_Excise(got, 2, 2);
    VA_Resize(wanted, 2);
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got),
              "Splicing too many elems truncates");

    VA_Excise(got, 0, 1);
    VA_Store(wanted, 0, VA_Delete(wanted, 1));
    VA_Resize(wanted, 1);
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got),
              "Excise first elem");

    DECREF(got);
    DECREF(wanted);
}

static void
test_Push_VArray(TestBatchRunner *runner) {
    VArray *wanted  = VA_new(0);
    VArray *got     = VA_new(0);
    VArray *scratch = VA_new(0);
    uint32_t i;

    for (i = 0; i < 4; i++) { VA_Push(wanted, (Obj*)Str_newf("%u32", i)); }
    for (i = 0; i < 2; i++) { VA_Push(got, (Obj*)Str_newf("%u32", i)); }
    for (i = 2; i < 4; i++) { VA_Push(scratch, (Obj*)Str_newf("%u32", i)); }

    VA_Push_VArray(got, scratch);
    TEST_TRUE(runner, VA_Equals(wanted, (Obj*)got), "Push_VArray");

    DECREF(wanted);
    DECREF(got);
    DECREF(scratch);
}

static void
test_Slice(TestBatchRunner *runner) {
    VArray *array = VA_new(0);
    for (uint32_t i = 0; i < 10; i++) { VA_Push(array, (Obj*)Str_newf("%u32", i)); }
    {
        VArray *slice = VA_Slice(array, 0, 10);
        TEST_TRUE(runner, VA_Equals(array, (Obj*)slice), "Slice entire array");
        DECREF(slice);
    }
    {
        VArray *slice = VA_Slice(array, 0, 11);
        TEST_TRUE(runner, VA_Equals(array, (Obj*)slice),
            "Exceed length");
        DECREF(slice);
    }
    {
        VArray *wanted = VA_new(0);
        VA_Push(wanted, (Obj*)Str_newf("9"));
        VArray *slice = VA_Slice(array, 9, 11);
        TEST_TRUE(runner, VA_Equals(slice, (Obj*)wanted),
            "Exceed length, start near end");
        DECREF(slice);
        DECREF(wanted);
    }
    {
        VArray *slice = VA_Slice(array, 0, 0);
        TEST_TRUE(runner, VA_Get_Size(slice) == 0, "empty slice");
        DECREF(slice);
    }
    {
        VArray *slice = VA_Slice(array, 20, 1);
        TEST_TRUE(runner, VA_Get_Size(slice) ==  0, "exceed offset");
        DECREF(slice);
    }
    {
        VArray *wanted = VA_new(0);
        VA_Push(wanted, (Obj*)Str_newf("9"));
        VArray *slice = VA_Slice(array, 9, UINT32_MAX - 1);
        TEST_TRUE(runner, VA_Get_Size(slice) == 1, "guard against overflow");
        DECREF(slice);
        DECREF(wanted);
    }
    DECREF(array);
}

static void
test_Clone_and_Shallow_Copy(TestBatchRunner *runner) {
    VArray *array = VA_new(0);
    VArray *twin;
    uint32_t i;

    for (i = 0; i < 10; i++) {
        VA_Push(array, (Obj*)Str_newf("%u32", i));
    }
    twin = VA_Shallow_Copy(array);
    TEST_TRUE(runner, VA_Equals(array, (Obj*)twin), "Shallow_Copy");
    TEST_TRUE(runner, VA_Fetch(array, 1) == VA_Fetch(twin, 1),
              "Shallow_Copy doesn't clone elements");
    DECREF(twin);

    twin = VA_Clone(array);
    TEST_TRUE(runner, VA_Equals(array, (Obj*)twin), "Clone");
    TEST_TRUE(runner, VA_Fetch(array, 1) != VA_Fetch(twin, 1),
              "Clone performs deep clone");

    DECREF(array);
    DECREF(twin);
}

void
TestVArray_Run_IMP(TestVArray *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 43);
    test_Equals(runner);
    test_Store_Fetch(runner);
    test_Push_Pop_Shift_Unshift(runner);
    test_Delete(runner);
    test_Resize(runner);
    test_Excise(runner);
    test_Push_VArray(runner);
    test_Slice(runner);
    test_Clone_and_Shallow_Copy(runner);
}


