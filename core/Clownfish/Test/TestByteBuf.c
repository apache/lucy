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

#include "Clownfish/Test/TestByteBuf.h"

#include "Clownfish/ByteBuf.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Clownfish/VTable.h"

TestByteBuf*
TestBB_new() {
    return (TestByteBuf*)VTable_Make_Obj(TESTBYTEBUF);
}

static void
test_Equals(TestBatchRunner *runner) {
    ByteBuf *wanted = BB_new_bytes("foo", 4); // Include terminating NULL.
    ByteBuf *got    = BB_new_bytes("foo", 4);

    TEST_TRUE(runner, BB_Equals(wanted, (Obj*)got), "Equals");
    TEST_INT_EQ(runner, BB_Hash_Sum(got), BB_Hash_Sum(wanted), "Hash_Sum");

    TEST_TRUE(runner, BB_Equals_Bytes(got, "foo", 4), "Equals_Bytes");
    TEST_FALSE(runner, BB_Equals_Bytes(got, "foo", 3),
               "Equals_Bytes spoiled by different size");
    TEST_FALSE(runner, BB_Equals_Bytes(got, "bar", 4),
               "Equals_Bytes spoiled by different content");

    BB_Set_Size(got, 3);
    TEST_FALSE(runner, BB_Equals(wanted, (Obj*)got),
               "Different size spoils Equals");
    TEST_FALSE(runner, BB_Hash_Sum(got) == BB_Hash_Sum(wanted),
               "Different size spoils Hash_Sum (probably -- at least this one)");

    BB_Mimic_Bytes(got, "bar", 4);
    TEST_INT_EQ(runner, BB_Get_Size(wanted), BB_Get_Size(got),
                "same length");
    TEST_FALSE(runner, BB_Equals(wanted, (Obj*)got),
               "Different content spoils Equals");

    DECREF(got);
    DECREF(wanted);
}

static void
test_Grow(TestBatchRunner *runner) {
    ByteBuf *bb = BB_new(1);
    TEST_INT_EQ(runner, BB_Get_Capacity(bb), 8,
                "Allocate in 8-byte increments");
    BB_Grow(bb, 9);
    TEST_INT_EQ(runner, BB_Get_Capacity(bb), 16,
                "Grow in 8-byte increments");
    DECREF(bb);
}

static void
test_Clone(TestBatchRunner *runner) {
    ByteBuf *bb = BB_new_bytes("foo", 3);
    ByteBuf *twin = BB_Clone(bb);
    TEST_TRUE(runner, BB_Equals(bb, (Obj*)twin), "Clone");
    DECREF(bb);
    DECREF(twin);
}

static void
test_compare(TestBatchRunner *runner) {
    ByteBuf *a = BB_new_bytes("foo\0a", 5);
    ByteBuf *b = BB_new_bytes("foo\0b", 5);

    BB_Set_Size(a, 4);
    BB_Set_Size(b, 4);
    TEST_INT_EQ(runner, BB_compare(&a, &b), 0,
                "BB_compare returns 0 for equal ByteBufs");

    BB_Set_Size(a, 3);
    TEST_TRUE(runner, BB_compare(&a, &b) < 0, "shorter ByteBuf sorts first");

    BB_Set_Size(a, 5);
    BB_Set_Size(b, 5);
    TEST_TRUE(runner, BB_compare(&a, &b) < 0,
              "NULL doesn't interfere with BB_compare");

    DECREF(a);
    DECREF(b);
}

static void
test_Mimic(TestBatchRunner *runner) {
    ByteBuf *a = BB_new_bytes("foo", 3);
    ByteBuf *b = BB_new(0);

    BB_Mimic(b, (Obj*)a);
    TEST_TRUE(runner, BB_Equals(a, (Obj*)b), "Mimic");

    BB_Mimic_Bytes(a, "bar", 4);
    TEST_TRUE(runner, strcmp(BB_Get_Buf(a), "bar") == 0,
              "Mimic_Bytes content");
    TEST_INT_EQ(runner, BB_Get_Size(a), 4, "Mimic_Bytes size");

    BB_Mimic(b, (Obj*)a);
    TEST_TRUE(runner, BB_Equals(a, (Obj*)b), "Mimic");

    DECREF(a);
    DECREF(b);
}

static void
test_Cat(TestBatchRunner *runner) {
    ByteBuf *wanted  = BB_new_bytes("foobar", 6);
    ByteBuf *got     = BB_new_bytes("foo", 3);
    ByteBuf *scratch = BB_new_bytes("bar", 3);

    BB_Cat(got, scratch);
    TEST_TRUE(runner, BB_Equals(wanted, (Obj*)got), "Cat");

    BB_Mimic_Bytes(wanted, "foobarbaz", 9);
    BB_Cat_Bytes(got, "baz", 3);
    TEST_TRUE(runner, BB_Equals(wanted, (Obj*)got), "Cat_Bytes");

    DECREF(scratch);
    DECREF(got);
    DECREF(wanted);
}

void
TestBB_run(TestByteBuf *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 21);
    test_Equals(runner);
    test_Grow(runner);
    test_Clone(runner);
    test_compare(runner);
    test_Mimic(runner);
    test_Cat(runner);
}


