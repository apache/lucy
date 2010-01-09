#define C_LUCY_TESTVARRAY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Object/TestVArray.h"

static CharBuf*
S_new_cb(const char *text)
{
    return CB_new_from_utf8(text, strlen(text));
}

static void
test_Equals(TestBatch *batch)
{
    VArray *array  = VA_new(0);
    VArray *other  = VA_new(0);

    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)other), 
        "Empty arrays are equal");

    VA_Push(array, INCREF(&EMPTY));
    ASSERT_FALSE(batch, VA_Equals(array, (Obj*)other), 
        "Add one elem and Equals returns false");

    VA_Push(other, INCREF(&EMPTY));
    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)other), 
        "Add a matching elem and Equals returns true");

    VA_Store(array, 2, INCREF(&EMPTY));
    ASSERT_FALSE(batch, VA_Equals(array, (Obj*)other), 
        "Add elem after a NULL and Equals returns false");

    VA_Store(other, 2, INCREF(&EMPTY));
    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)other), 
        "Empty elems don't spoil Equals");

    VA_Store(other, 2, INCREF(UNDEF));
    ASSERT_FALSE(batch, VA_Equals(array, (Obj*)other), 
        "Non-matching value spoils Equals");

    VA_Splice(array, 1, 2); /* removes empty elems */
    VA_Delete(other, 1);    /* leaves NULL in place of deleted elem */
    VA_Delete(other, 2);
    ASSERT_FALSE(batch, VA_Equals(array, (Obj*)other), 
        "Empty trailing elements spoil Equals");

    DECREF(array);
    DECREF(other);
}

static void
test_Store_Fetch(TestBatch *batch)
{
    VArray *array = VA_new(0);
    CharBuf *elem;

    ASSERT_TRUE(batch, VA_Fetch(array, 2) == NULL, "Fetch beyond end");

    VA_Store(array, 2, (Obj*)CB_newf("foo"));
    elem = (CharBuf*)CERTIFY(VA_Fetch(array, 2), CHARBUF);
    ASSERT_INT_EQ(batch, 3, VA_Get_Size(array), "Store updates size");
    ASSERT_TRUE(batch, CB_Equals_Str(elem, "foo", 3), "Store");

    INCREF(elem);
    ASSERT_INT_EQ(batch, 2, CB_Get_RefCount(elem), 
        "start with refcount of 2");
    VA_Store(array, 2, (Obj*)CB_newf("bar"));
    ASSERT_INT_EQ(batch, 1, CB_Get_RefCount(elem), 
        "Displacing elem via Store updates refcount");
    DECREF(elem);
    elem = (CharBuf*)CERTIFY(VA_Fetch(array, 2), CHARBUF);
    ASSERT_TRUE(batch, CB_Equals_Str(elem, "bar", 3), "Store displacement");

    DECREF(array);
}

static void
test_Push_Pop_Shift_Unshift(TestBatch *batch)
{
    VArray *array = VA_new(0);
    CharBuf *elem;

    ASSERT_INT_EQ(batch, VA_Get_Size(array), 0, "size starts at 0");
    VA_Push(array, (Obj*)CB_newf("a"));
    VA_Push(array, (Obj*)CB_newf("b"));
    VA_Push(array, (Obj*)CB_newf("c"));

    ASSERT_INT_EQ(batch, VA_Get_Size(array), 3, "size after Push");
    ASSERT_TRUE(batch, NULL != CERTIFY(VA_Fetch(array, 2), CHARBUF), "Push");

    elem = (CharBuf*)CERTIFY(VA_Shift(array), CHARBUF);
    ASSERT_TRUE(batch, CB_Equals_Str(elem, "a", 1), "Shift");
    ASSERT_INT_EQ(batch, VA_Get_Size(array), 2, "size after Shift");
    DECREF(elem);

    elem = (CharBuf*)CERTIFY(VA_Pop(array), CHARBUF);
    ASSERT_TRUE(batch, CB_Equals_Str(elem, "c", 1), "Pop");
    ASSERT_INT_EQ(batch, VA_Get_Size(array), 1, "size after Pop");
    DECREF(elem);

    VA_Unshift(array, (Obj*)CB_newf("foo"));
    elem = (CharBuf*)CERTIFY(VA_Fetch(array, 0), CHARBUF);
    ASSERT_TRUE(batch, CB_Equals_Str(elem, "foo", 3), "Unshift");
    ASSERT_INT_EQ(batch, VA_Get_Size(array), 2, "size after Shift");

    DECREF(array);
}

static void
test_Delete(TestBatch *batch)
{
    VArray *wanted  = VA_new(5);
    VArray *got     = VA_new(5);
    u32_t i;

    for (i = 0; i < 5; i++) { VA_Push(got, (Obj*)CB_newf("%u32", i)); }
    VA_Store(wanted, 0, (Obj*)CB_newf("0", i));
    VA_Store(wanted, 1, (Obj*)CB_newf("1", i));
    VA_Store(wanted, 4, (Obj*)CB_newf("4", i));
    DECREF(VA_Delete(got, 2));
    DECREF(VA_Delete(got, 3));
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), "Delete");

    DECREF(wanted);
    DECREF(got);
}

static void
test_Resize(TestBatch *batch)
{
    VArray *array = VA_new(3);
    u32_t i;

    for (i = 0; i < 2; i++) { VA_Push(array, (Obj*)CB_newf("%u32", i)); }
    ASSERT_INT_EQ(batch, VA_Get_Capacity(array), 3, "Start with capacity 3");

    VA_Resize(array, 4);
    ASSERT_INT_EQ(batch, VA_Get_Size(array), 4, "Resize up");
    ASSERT_INT_EQ(batch, VA_Get_Capacity(array), 4, 
        "Resize changes capacity");

    VA_Resize(array, 2);
    ASSERT_INT_EQ(batch, VA_Get_Size(array), 2, "Resize down");
    ASSERT_TRUE(batch, VA_Fetch(array, 2) == NULL, "Resize down zaps elem");

    DECREF(array);
}

static void
test_Splice(TestBatch *batch)
{
    VArray *wanted = VA_new(5);
    VArray *got    = VA_new(5);
    u32_t   i;

    for (i = 0; i < 5; i++) { VA_Push(wanted, (Obj*)CB_newf("%u32", i)); }
    for (i = 0; i < 5; i++) { VA_Push(got,    (Obj*)CB_newf("%u32", i)); }

    VA_Splice(got, 7, 1);
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), 
        "Splice outside of range is no-op" );

    VA_Splice(got, 2, 2);
    DECREF(VA_Delete(wanted, 2));
    DECREF(VA_Delete(wanted, 3));
    VA_Store(wanted, 2, VA_Delete(wanted, 4));
    VA_Resize(wanted, 3); 
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), 
        "Splice multiple elems" );

    VA_Splice(got, 2, 2);
    VA_Resize(wanted, 2);
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), 
        "Splicing too many elems truncates" );

    VA_Splice(got, 0, 1);
    VA_Store(wanted, 0, VA_Delete(wanted, 1));
    VA_Resize(wanted, 1);
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), 
        "Splice first elem" );

    DECREF(got);
    DECREF(wanted);
}

static void
test_Push_VArray(TestBatch *batch)
{
    VArray *wanted  = VA_new(0);
    VArray *got     = VA_new(0);
    VArray *scratch = VA_new(0);
    u32_t i;

    for (i = 0; i < 4; i++) { VA_Push(wanted, (Obj*)CB_newf("%u32", i)); }
    for (i = 0; i < 2; i++) { VA_Push(got, (Obj*)CB_newf("%u32", i)); }
    for (i = 2; i < 4; i++) { VA_Push(scratch, (Obj*)CB_newf("%u32", i)); }

    VA_Push_VArray(got, scratch);
    ASSERT_TRUE(batch, VA_Equals(wanted, (Obj*)got), "Push_VArray");

    DECREF(wanted);
    DECREF(got);
    DECREF(scratch);
}

static void
test_Clone_and_Shallow_Copy(TestBatch *batch)
{
    VArray *array  = VA_new(0);
    VArray *evil_twin;
    u32_t i;

    for (i = 0; i < 10; i++) {
        VA_Push(array, (Obj*)CB_newf("%u32", i));
    }
    evil_twin = VA_Shallow_Copy(array);
    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)evil_twin), "Shallow_Copy");
    ASSERT_TRUE(batch, VA_Fetch(array, 1) == VA_Fetch(evil_twin, 1), 
        "Shallow_Copy doesn't clone elements");
    DECREF(evil_twin);

    evil_twin = VA_Clone(array);
    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)evil_twin), "Clone");
    ASSERT_TRUE(batch, VA_Fetch(array, 1) != VA_Fetch(evil_twin, 1), 
        "Clone performs deep clone");

    DECREF(array);
    DECREF(evil_twin);
}

static void
test_Dump_and_Load(TestBatch *batch)
{
    VArray *array  = VA_new(0);
    Obj    *dump;
    VArray *loaded;

    VA_Push(array, (Obj*)S_new_cb("foo"));
    dump = (Obj*)VA_Dump(array);
    loaded = (VArray*)Obj_Load(dump, dump);
    ASSERT_TRUE(batch, VA_Equals(array, (Obj*)loaded), 
        "Dump => Load round trip");

    DECREF(array);
    DECREF(dump);
    DECREF(loaded);
}

static void
test_serialization(TestBatch *batch)
{
    VArray *array  = VA_new(0);
    VArray *dupe;
    VA_Store(array, 1, (Obj*)CB_newf("foo"));
    VA_Store(array, 3, (Obj*)CB_newf("bar"));
    dupe = (VArray*)TestUtils_freeze_thaw((Obj*)array);
    ASSERT_TRUE(batch, dupe && VA_Equals(array, (Obj*)dupe), 
        "Round trip through FREEZE/THAW");
    DECREF(dupe);
    DECREF(array);
}

void
TestVArray_run_tests()
{
    TestBatch *batch = TestBatch_new(39);

    TestBatch_Plan(batch);

    test_Equals(batch);
    test_Store_Fetch(batch);
    test_Push_Pop_Shift_Unshift(batch);
    test_Delete(batch);
    test_Resize(batch);
    test_Splice(batch);
    test_Push_VArray(batch);
    test_Clone_and_Shallow_Copy(batch);
    test_Dump_and_Load(batch);
    test_serialization(batch);

    DECREF(batch);
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

