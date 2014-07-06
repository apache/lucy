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

#define C_TESTLUCY_TESTBITVECTOR
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Object/TestBitVector.h"

TestBitVector*
TestBitVector_new() {
    return (TestBitVector*)Class_Make_Obj(TESTBITVECTOR);
}

static void
test_Set_and_Get(TestBatchRunner *runner) {
    unsigned i, max;
    const uint32_t  three     = 3;
    const uint32_t  seventeen = 17;
    BitVector      *bit_vec   = BitVec_new(8);

    BitVec_Set(bit_vec, three);
    TEST_TRUE(runner, BitVec_Get_Capacity(bit_vec) < seventeen,
              "set below cap");
    BitVec_Set(bit_vec, seventeen);
    TEST_TRUE(runner, BitVec_Get_Capacity(bit_vec) > seventeen,
              "set above cap causes BitVector to grow");

    for (i = 0, max = BitVec_Get_Capacity(bit_vec); i < max; i++) {
        if (i == three || i == seventeen) {
            TEST_TRUE(runner, BitVec_Get(bit_vec, i), "set/get %d", i);
        }
        else {
            TEST_FALSE(runner, BitVec_Get(bit_vec, i), "get %d", i);
        }
    }
    TEST_FALSE(runner, BitVec_Get(bit_vec, i), "out of range get");

    DECREF(bit_vec);
}

static void
test_Flip(TestBatchRunner *runner) {
    BitVector *bit_vec = BitVec_new(0);
    int i;

    for (i = 0; i <= 20; i++) { BitVec_Flip(bit_vec, i); }
    for (i = 0; i <= 20; i++) {
        TEST_TRUE(runner, BitVec_Get(bit_vec, i), "flip on %d", i);
    }
    TEST_FALSE(runner, BitVec_Get(bit_vec, i), "no flip %d", i);
    for (i = 0; i <= 20; i++) { BitVec_Flip(bit_vec, i); }
    for (i = 0; i <= 20; i++) {
        TEST_FALSE(runner, BitVec_Get(bit_vec, i), "flip off %d", i);
    }
    TEST_FALSE(runner, BitVec_Get(bit_vec, i), "still no flip %d", i);

    DECREF(bit_vec);
}

static void
test_Flip_Block_ascending(TestBatchRunner *runner) {
    BitVector *bit_vec = BitVec_new(0);
    int i;

    for (i = 0; i <= 20; i++) {
        BitVec_Flip_Block(bit_vec, i, 21 - i);
    }

    for (i = 0; i <= 20; i++) {
        if (i % 2 == 0) {
            TEST_TRUE(runner, BitVec_Get(bit_vec, i),
                      "Flip_Block ascending %d", i);
        }
        else {
            TEST_FALSE(runner, BitVec_Get(bit_vec, i),
                       "Flip_Block ascending %d", i);
        }
    }

    DECREF(bit_vec);
}

static void
test_Flip_Block_descending(TestBatchRunner *runner) {
    BitVector *bit_vec = BitVec_new(0);
    int i;

    for (i = 19; i >= 0; i--) {
        BitVec_Flip_Block(bit_vec, 1, i);
    }

    for (i = 0; i <= 20; i++) {
        if (i % 2) {
            TEST_TRUE(runner, BitVec_Get(bit_vec, i),
                      "Flip_Block descending %d", i);
        }
        else {
            TEST_FALSE(runner, BitVec_Get(bit_vec, i),
                       "Flip_Block descending %d", i);
        }
    }

    DECREF(bit_vec);
}

static void
test_Flip_Block_bulk(TestBatchRunner *runner) {
    int32_t offset;

    for (offset = 0; offset <= 17; offset++) {
        int32_t len;
        for (len = 0; len <= 17; len++) {
            int i;
            int upper = offset + len - 1;
            BitVector *bit_vec = BitVec_new(0);

            BitVec_Flip_Block(bit_vec, offset, len);
            for (i = 0; i <= 17; i++) {
                if (i >= offset && i <= upper) {
                    if (!BitVec_Get(bit_vec, i)) { break; }
                }
                else {
                    if (BitVec_Get(bit_vec, i)) { break; }
                }
            }
            TEST_INT_EQ(runner, i, 18, "Flip_Block(%d, %d)", offset, len);

            DECREF(bit_vec);
        }
    }
}

static void
test_Mimic(TestBatchRunner *runner) {
    int foo;

    for (foo = 0; foo <= 17; foo++) {
        int bar;
        for (bar = 0; bar <= 17; bar++) {
            int i;
            BitVector *foo_vec = BitVec_new(0);
            BitVector *bar_vec = BitVec_new(0);

            BitVec_Set(foo_vec, foo);
            BitVec_Set(bar_vec, bar);
            BitVec_Mimic(foo_vec, (Obj*)bar_vec);

            for (i = 0; i <= 17; i++) {
                if (BitVec_Get(foo_vec, i) && i != bar) { break; }
            }
            TEST_INT_EQ(runner, i, 18, "Mimic(%d, %d)", foo, bar);

            DECREF(foo_vec);
            DECREF(bar_vec);
        }
    }
}

static BitVector*
S_create_set(int set_num) {
    int i;
    int nums_1[] = { 1, 2, 3, 10, 20, 30, 0 };
    int nums_2[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10,
                     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 0
                   };
    int *nums = set_num == 1 ? nums_1 : nums_2;
    BitVector *bit_vec = BitVec_new(31);
    for (i = 0; nums[i] != 0; i++) {
        BitVec_Set(bit_vec, nums[i]);
    }
    return bit_vec;
}

#define OP_OR 1
#define OP_XOR 2
#define OP_AND 3
#define OP_AND_NOT 4
static int
S_verify_logical_op(BitVector *bit_vec, BitVector *set_1, BitVector *set_2,
                    int op) {
    int i;

    for (i = 0; i < 50; i++) {
        bool wanted;

        switch (op) {
            case OP_OR:
                wanted = BitVec_Get(set_1, i) || BitVec_Get(set_2, i);
                break;
            case OP_XOR:
                wanted = (!BitVec_Get(set_1, i)) != (!BitVec_Get(set_2, i));
                break;
            case OP_AND:
                wanted = BitVec_Get(set_1, i) && BitVec_Get(set_2, i);
                break;
            case OP_AND_NOT:
                wanted = BitVec_Get(set_1, i) && (!BitVec_Get(set_2, i));
                break;
            default:
                wanted = false;
                THROW(ERR, "unknown op: %d", op);
        }

        if (BitVec_Get(bit_vec, i) != wanted) { break; }
    }

    return i;
}

static void
test_Or(TestBatchRunner *runner) {
    BitVector *smaller = S_create_set(1);
    BitVector *larger  = S_create_set(2);
    BitVector *set_1   = S_create_set(1);
    BitVector *set_2   = S_create_set(2);

    BitVec_Or(smaller, set_2);
    TEST_INT_EQ(runner, S_verify_logical_op(smaller, set_1, set_2, OP_OR),
                50, "OR with self smaller than other");
    BitVec_Or(larger, set_1);
    TEST_INT_EQ(runner, S_verify_logical_op(larger, set_1, set_2, OP_OR),
                50, "OR with other smaller than self");

    DECREF(smaller);
    DECREF(larger);
    DECREF(set_1);
    DECREF(set_2);
}

static void
test_Xor(TestBatchRunner *runner) {
    BitVector *smaller = S_create_set(1);
    BitVector *larger  = S_create_set(2);
    BitVector *set_1   = S_create_set(1);
    BitVector *set_2   = S_create_set(2);

    BitVec_Xor(smaller, set_2);
    TEST_INT_EQ(runner, S_verify_logical_op(smaller, set_1, set_2, OP_XOR),
                50, "XOR with self smaller than other");
    BitVec_Xor(larger, set_1);
    TEST_INT_EQ(runner, S_verify_logical_op(larger, set_1, set_2, OP_XOR),
                50, "XOR with other smaller than self");

    DECREF(smaller);
    DECREF(larger);
    DECREF(set_1);
    DECREF(set_2);
}

static void
test_And(TestBatchRunner *runner) {
    BitVector *smaller = S_create_set(1);
    BitVector *larger  = S_create_set(2);
    BitVector *set_1   = S_create_set(1);
    BitVector *set_2   = S_create_set(2);

    BitVec_And(smaller, set_2);
    TEST_INT_EQ(runner, S_verify_logical_op(smaller, set_1, set_2, OP_AND),
                50, "AND with self smaller than other");
    BitVec_And(larger, set_1);
    TEST_INT_EQ(runner, S_verify_logical_op(larger, set_1, set_2, OP_AND),
                50, "AND with other smaller than self");

    DECREF(smaller);
    DECREF(larger);
    DECREF(set_1);
    DECREF(set_2);
}

static void
test_And_Not(TestBatchRunner *runner) {
    BitVector *smaller = S_create_set(1);
    BitVector *larger  = S_create_set(2);
    BitVector *set_1   = S_create_set(1);
    BitVector *set_2   = S_create_set(2);

    BitVec_And_Not(smaller, set_2);
    TEST_INT_EQ(runner,
                S_verify_logical_op(smaller, set_1, set_2, OP_AND_NOT),
                50, "AND_NOT with self smaller than other");
    BitVec_And_Not(larger, set_1);
    TEST_INT_EQ(runner,
                S_verify_logical_op(larger, set_2, set_1, OP_AND_NOT),
                50, "AND_NOT with other smaller than self");

    DECREF(smaller);
    DECREF(larger);
    DECREF(set_1);
    DECREF(set_2);
}

static void
test_Count(TestBatchRunner *runner) {
    int i;
    int shuffled[64];
    BitVector *bit_vec = BitVec_new(64);

    for (i = 0; i < 64; i++) { shuffled[i] = i; }
    for (i = 0; i < 64; i++) {
        int shuffle_pos = rand() % 64;
        int temp = shuffled[shuffle_pos];
        shuffled[shuffle_pos] = shuffled[i];
        shuffled[i] = temp;
    }
    for (i = 0; i < 64; i++) {
        BitVec_Set(bit_vec, shuffled[i]);
        if (BitVec_Count(bit_vec) != (uint32_t)(i + 1)) { break; }
    }
    TEST_INT_EQ(runner, i, 64, "Count() returns the right number of bits");

    DECREF(bit_vec);
}

static void
test_Next_Hit(TestBatchRunner *runner) {
    int i;

    for (i = 24; i <= 33; i++) {
        int probe;
        BitVector *bit_vec = BitVec_new(64);
        BitVec_Set(bit_vec, i);
        TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 0), i,
                    "Next_Hit for 0 is %d", i);
        TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 0), i,
                    "Next_Hit for 1 is %d", i);
        for (probe = 15; probe <= i; probe++) {
            TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, probe), i,
                        "Next_Hit for %d is %d", probe, i);
        }
        for (probe = i + 1; probe <= i + 9; probe++) {
            TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, probe), -1,
                        "no Next_Hit for %d when max is %d", probe, i);
        }
        DECREF(bit_vec);
    }
}

static void
test_Clear_All(TestBatchRunner *runner) {
    BitVector *bit_vec = BitVec_new(64);
    BitVec_Flip_Block(bit_vec, 0, 63);
    BitVec_Clear_All(bit_vec);
    TEST_INT_EQ(runner, BitVec_Next_Hit(bit_vec, 0), -1, "Clear_All");
    DECREF(bit_vec);
}

static void
test_Clone(TestBatchRunner *runner) {
    int i;
    BitVector *self = BitVec_new(30);
    BitVector *twin;

    BitVec_Set(self, 2);
    BitVec_Set(self, 3);
    BitVec_Set(self, 10);
    BitVec_Set(self, 20);

    twin = BitVec_Clone(self);
    for (i = 0; i < 50; i++) {
        if (BitVec_Get(self, i) != BitVec_Get(twin, i)) { break; }
    }
    TEST_INT_EQ(runner, i, 50, "Clone");
    TEST_INT_EQ(runner, BitVec_Count(twin), 4, "clone Count");

    DECREF(self);
    DECREF(twin);
}

static int
S_compare_u64s(void *context, const void *va, const void *vb) {
    uint64_t a = *(uint64_t*)va;
    uint64_t b = *(uint64_t*)vb;
    UNUSED_VAR(context);
    return a == b ? 0 : a < b ? -1 : 1;
}

static void
test_To_Array(TestBatchRunner *runner) {
    uint64_t  *source_ints = TestUtils_random_u64s(NULL, 20, 0, 200);
    BitVector *bit_vec = BitVec_new(0);
    I32Array  *array;
    long       num_unique = 0;
    long       i;

    // Unique the random ints.
    Sort_quicksort(source_ints, 20, sizeof(uint64_t),
                   S_compare_u64s, NULL);
    for (i = 0; i < 19; i++) {
        if (source_ints[i] != source_ints[i + 1]) {
            source_ints[num_unique] = source_ints[i];
            num_unique++;
        }
    }

    // Set bits.
    for (i = 0; i < num_unique; i++) {
        BitVec_Set(bit_vec, (uint32_t)source_ints[i]);
    }

    // Create the array and compare it to the source.
    array = BitVec_To_Array(bit_vec);
    for (i = 0; i < num_unique; i++) {
        if (I32Arr_Get(array, i) != (int32_t)source_ints[i]) { break; }
    }
    TEST_INT_EQ(runner, i, num_unique, "To_Array (%ld == %ld)", i,
                num_unique);

    DECREF(array);
    DECREF(bit_vec);
    FREEMEM(source_ints);
}


// Valgrind only - detect off-by-one error.
static void
test_off_by_one_error() {
    int cap;
    for (cap = 5; cap <= 24; cap++) {
        BitVector *bit_vec = BitVec_new(cap);
        BitVec_Set(bit_vec, cap - 2);
        DECREF(bit_vec);
    }
}

void
TestBitVector_Run_IMP(TestBitVector *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1029);
    test_Set_and_Get(runner);
    test_Flip(runner);
    test_Flip_Block_ascending(runner);
    test_Flip_Block_descending(runner);
    test_Flip_Block_bulk(runner);
    test_Mimic(runner);
    test_Or(runner);
    test_Xor(runner);
    test_And(runner);
    test_And_Not(runner);
    test_Count(runner);
    test_Next_Hit(runner);
    test_Clear_All(runner);
    test_Clone(runner);
    test_To_Array(runner);
    test_off_by_one_error();
}


