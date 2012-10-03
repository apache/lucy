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

#define C_LUCY_TESTBYTEBUF
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Object/TestByteBuf.h"

static void
test_Equals(TestBatch *batch) {
    ByteBuf *wanted = BB_new_bytes("foo", 4); // Include terminating NULL.
    ByteBuf *got    = BB_new_bytes("foo", 4);

    TEST_TRUE(batch, BB_Equals(wanted, (Obj*)got), "Equals");
    TEST_INT_EQ(batch, BB_Hash_Sum(got), BB_Hash_Sum(wanted), "Hash_Sum");

    TEST_TRUE(batch, BB_Equals_Bytes(got, "foo", 4), "Equals_Bytes");
    TEST_FALSE(batch, BB_Equals_Bytes(got, "foo", 3),
               "Equals_Bytes spoiled by different size");
    TEST_FALSE(batch, BB_Equals_Bytes(got, "bar", 4),
               "Equals_Bytes spoiled by different content");

    BB_Set_Size(got, 3);
    TEST_FALSE(batch, BB_Equals(wanted, (Obj*)got),
               "Different size spoils Equals");
    TEST_FALSE(batch, BB_Hash_Sum(got) == BB_Hash_Sum(wanted),
               "Different size spoils Hash_Sum (probably -- at least this one)");

    BB_Mimic_Bytes(got, "bar", 4);
    TEST_INT_EQ(batch, BB_Get_Size(wanted), BB_Get_Size(got),
                "same length");
    TEST_FALSE(batch, BB_Equals(wanted, (Obj*)got),
               "Different content spoils Equals");

    DECREF(got);
    DECREF(wanted);
}

static void
test_Grow(TestBatch *batch) {
    ByteBuf *bb = BB_new(1);
    TEST_INT_EQ(batch, BB_Get_Capacity(bb), 8,
                "Allocate in 8-byte increments");
    BB_Grow(bb, 9);
    TEST_INT_EQ(batch, BB_Get_Capacity(bb), 16,
                "Grow in 8-byte increments");
    DECREF(bb);
}

static void
test_Clone(TestBatch *batch) {
    ByteBuf *bb = BB_new_bytes("foo", 3);
    ByteBuf *twin = BB_Clone(bb);
    TEST_TRUE(batch, BB_Equals(bb, (Obj*)twin), "Clone");
    DECREF(bb);
    DECREF(twin);
}

static void
test_compare(TestBatch *batch) {
    ByteBuf *a = BB_new_bytes("foo\0a", 5);
    ByteBuf *b = BB_new_bytes("foo\0b", 5);

    BB_Set_Size(a, 4);
    BB_Set_Size(b, 4);
    TEST_INT_EQ(batch, BB_compare(&a, &b), 0,
                "BB_compare returns 0 for equal ByteBufs");

    BB_Set_Size(a, 3);
    TEST_TRUE(batch, BB_compare(&a, &b) < 0, "shorter ByteBuf sorts first");

    BB_Set_Size(a, 5);
    BB_Set_Size(b, 5);
    TEST_TRUE(batch, BB_compare(&a, &b) < 0,
              "NULL doesn't interfere with BB_compare");

    DECREF(a);
    DECREF(b);
}

static void
test_Mimic(TestBatch *batch) {
    ByteBuf *a = BB_new_bytes("foo", 3);
    ByteBuf *b = BB_new(0);

    BB_Mimic(b, (Obj*)a);
    TEST_TRUE(batch, BB_Equals(a, (Obj*)b), "Mimic");

    BB_Mimic_Bytes(a, "bar", 4);
    TEST_TRUE(batch, strcmp(BB_Get_Buf(a), "bar") == 0,
              "Mimic_Bytes content");
    TEST_INT_EQ(batch, BB_Get_Size(a), 4, "Mimic_Bytes size");

    BB_Mimic(b, (Obj*)a);
    TEST_TRUE(batch, BB_Equals(a, (Obj*)b), "Mimic");

    DECREF(a);
    DECREF(b);
}

static void
test_Cat(TestBatch *batch) {
    ByteBuf *wanted  = BB_new_bytes("foobar", 6);
    ByteBuf *got     = BB_new_bytes("foo", 3);
    ByteBuf *scratch = BB_new_bytes("bar", 3);

    BB_Cat(got, scratch);
    TEST_TRUE(batch, BB_Equals(wanted, (Obj*)got), "Cat");

    BB_Mimic_Bytes(wanted, "foobarbaz", 9);
    BB_Cat_Bytes(got, "baz", 3);
    TEST_TRUE(batch, BB_Equals(wanted, (Obj*)got), "Cat_Bytes");

    DECREF(scratch);
    DECREF(got);
    DECREF(wanted);
}

static void
test_serialization(TestBatch *batch) {
    ByteBuf *wanted = BB_new_bytes("foobar", 6);
    ByteBuf *got    = (ByteBuf*)TestUtils_freeze_thaw((Obj*)wanted);
    TEST_TRUE(batch, got && BB_Equals(wanted, (Obj*)got),
              "Serialization round trip");
    DECREF(wanted);
    DECREF(got);
}

void
TestBB_run_tests() {
    TestBatch *batch = TestBatch_new(22);
    TestBatch_Plan(batch);

    test_Equals(batch);
    test_Grow(batch);
    test_Clone(batch);
    test_compare(batch);
    test_Mimic(batch);
    test_Cat(batch);
    test_serialization(batch);

    DECREF(batch);
}


