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

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include "Lucy/Test/Util/TestSortExternal.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Util/BBSortEx.h"
#include "Lucy/Util/SortExternal.h"

static ByteBuf *a_bb;
static ByteBuf *b_bb;
static ByteBuf *c_bb;
static ByteBuf *d_bb;
static ByteBuf *x_bb;
static ByteBuf *y_bb;
static ByteBuf *z_bb;

TestSortExternal*
TestSortExternal_new() {
    return (TestSortExternal*)Class_Make_Obj(TESTSORTEXTERNAL);
}

static void
S_init_bytebufs() {
    a_bb = BB_new_bytes("a", 1);
    b_bb = BB_new_bytes("b", 1);
    c_bb = BB_new_bytes("c", 1);
    d_bb = BB_new_bytes("d", 1);
    x_bb = BB_new_bytes("x", 1);
    y_bb = BB_new_bytes("y", 1);
    z_bb = BB_new_bytes("z", 1);
}

static void
S_destroy_bytebufs() {
    DECREF(a_bb);
    DECREF(b_bb);
    DECREF(c_bb);
    DECREF(d_bb);
    DECREF(x_bb);
    DECREF(y_bb);
    DECREF(z_bb);
}

static void
test_bbsortex(TestBatchRunner *runner) {
    BBSortEx *sortex = BBSortEx_new(4, NULL);

    BBSortEx_Feed(sortex, INCREF(c_bb));
    TEST_INT_EQ(runner, BBSortEx_Buffer_Count(sortex), 1,
                "feed elem into cache");

    BBSortEx_Feed(sortex, INCREF(b_bb));
    BBSortEx_Feed(sortex, INCREF(d_bb));
    BBSortEx_Sort_Buffer(sortex);

    {
        VArray *cache  = BBSortEx_Peek_Cache(sortex);
        VArray *wanted = VA_new(3);
        VA_Push(wanted, INCREF(b_bb));
        VA_Push(wanted, INCREF(c_bb));
        VA_Push(wanted, INCREF(d_bb));
        TEST_TRUE(runner, VA_Equals(cache, (Obj*)wanted), "sort cache");
        DECREF(wanted);
        DECREF(cache);
    }

    BBSortEx_Feed(sortex, INCREF(a_bb));
    TEST_INT_EQ(runner, BBSortEx_Buffer_Count(sortex), 0,
                "cache flushed automatically when mem_thresh crossed");
    TEST_INT_EQ(runner, BBSortEx_Get_Num_Runs(sortex), 1, "run added");

    VArray *external = VA_new(3);
    VA_Push(external, INCREF(x_bb));
    VA_Push(external, INCREF(y_bb));
    VA_Push(external, INCREF(z_bb));
    BBSortEx *run = BBSortEx_new(0x1000000, external);
    BBSortEx_Add_Run(sortex, (SortExternal*)run);
    BBSortEx_Flip(sortex);

    {
        VArray *got = VA_new(7);
        Obj *object;
        while (NULL != (object = BBSortEx_Fetch(sortex))) {
            VA_Push(got, object);
        }

        VArray *wanted = VA_new(7);
        VA_Push(wanted, INCREF(a_bb));
        VA_Push(wanted, INCREF(b_bb));
        VA_Push(wanted, INCREF(c_bb));
        VA_Push(wanted, INCREF(d_bb));
        VA_Push(wanted, INCREF(x_bb));
        VA_Push(wanted, INCREF(y_bb));
        VA_Push(wanted, INCREF(z_bb));

        TEST_TRUE(runner, VA_Equals(got, (Obj*)wanted), "Add_Run");

        DECREF(wanted);
        DECREF(got);
    }

    DECREF(external);
    DECREF(sortex);
}

static void
test_clear_buffer(TestBatchRunner *runner) {
    BBSortEx *sortex = BBSortEx_new(4, NULL);

    BBSortEx_Feed(sortex, INCREF(c_bb));
    BBSortEx_Clear_Buffer(sortex);
    TEST_INT_EQ(runner, BBSortEx_Buffer_Count(sortex), 0, "Clear_Buffer");

    BBSortEx_Feed(sortex, INCREF(b_bb));
    BBSortEx_Feed(sortex, INCREF(a_bb));
    BBSortEx_Flush(sortex);
    BBSortEx_Flip(sortex);
    Obj *object = BBSortEx_Peek(sortex);
    TEST_TRUE(runner, BB_Equals(a_bb, object), "Peek");

    VArray *got = VA_new(2);
    while (NULL != (object = BBSortEx_Fetch(sortex))) {
        VA_Push(got, object);
    }
    VArray *wanted = VA_new(2);
    VA_Push(wanted, INCREF(a_bb));
    VA_Push(wanted, INCREF(b_bb));
    TEST_TRUE(runner, VA_Equals(got, (Obj*)wanted),
              "elements cleared via Clear_Buffer truly cleared");

    DECREF(wanted);
    DECREF(got);
    DECREF(sortex);
}

static void
S_test_sort(TestBatchRunner *runner, VArray *bytebufs, uint32_t mem_thresh,
            const char *test_name) {
    int        size     = (int)VA_Get_Size(bytebufs);
    BBSortEx  *sortex   = BBSortEx_new(mem_thresh, NULL);
    ByteBuf  **shuffled = (ByteBuf**)MALLOCATE(size * sizeof(ByteBuf*));

    for (int i = 0; i < size; ++i) {
        shuffled[i] = (ByteBuf*)CERTIFY(VA_Fetch(bytebufs, i), BYTEBUF);
    }
    for (int i = size - 1; i > 0; --i) {
        int shuffle_pos = rand() % (i + 1);
        ByteBuf *temp = shuffled[shuffle_pos];
        shuffled[shuffle_pos] = shuffled[i];
        shuffled[i] = temp;
    }
    for (int i = 0; i < size; ++i) {
        BBSortEx_Feed(sortex, INCREF(shuffled[i]));
    }

    BBSortEx_Flip(sortex);
    VArray *got = VA_new(size);
    Obj *object;
    while (NULL != (object = BBSortEx_Fetch(sortex))) {
        VA_Push(got, object);
    }
    TEST_TRUE(runner, VA_Equals(got, (Obj*)bytebufs), test_name);

    FREEMEM(shuffled);
    DECREF(got);
    DECREF(sortex);
}

static void
S_test_sort_letters(TestBatchRunner *runner, const char *letters,
                    uint32_t mem_thresh, const char *test_name) {
    size_t  num_letters = strlen(letters);
    VArray *bytebufs    = VA_new(num_letters);

    for (size_t i = 0; i < num_letters; ++i) {
        char ch = letters[i];
        size_t size = ch == '_' ? 0 : 1;
        ByteBuf *bytebuf = BB_new_bytes(&ch, size);
        VA_Push(bytebufs, (Obj*)bytebuf);
    }

    S_test_sort(runner, bytebufs, mem_thresh, test_name);

    DECREF(bytebufs);
}

static void
test_sort_letters(TestBatchRunner *runner) {
    S_test_sort_letters(runner, "abcdefghijklmnopqrstuvwxyz", 0x1000000,
                        "sort letters");
    S_test_sort_letters(runner, "aaabcdxxxxxxyy", 0x1000000,
                        "sort repeated letters");
    S_test_sort_letters(runner, "__abcdefghijklmnopqrstuvwxyz", 0x1000000,
                        "sort letters and empty strings");
    S_test_sort_letters(runner, "abcdefghijklmnopqrstuvwxyz", 30,
                        "... with an absurdly low mem_thresh");
    S_test_sort_letters(runner, "abcdefghijklmnopqrstuvwxyz", 1,
                        "... with an even lower mem_thresh");
}

static void
test_sort_nothing(TestBatchRunner *runner) {
    BBSortEx *sortex = BBSortEx_new(0x1000000, NULL);
    BBSortEx_Flip(sortex);
    TEST_TRUE(runner, BBSortEx_Fetch(sortex) == NULL,
              "Sorting nothing returns undef");
    DECREF(sortex);
}

static void
test_sort_packed_ints(TestBatchRunner *runner) {
    size_t  num_ints = 11001;
    VArray *bytebufs = VA_new(num_ints);

    for (uint32_t i = 0; i < num_ints; ++i) {
        char buf[4];
        buf[0] = i >> 24;
        buf[1] = i >> 16;
        buf[2] = i >> 8;
        buf[3] = i;
        ByteBuf *bytebuf = BB_new_bytes(&buf, 4);
        VA_Push(bytebufs, (Obj*)bytebuf);
    }

    S_test_sort(runner, bytebufs, 5000, "Sorting packed integers...");

    DECREF(bytebufs);
}

static void
test_sort_random_strings(TestBatchRunner *runner) {
    size_t  num_strings = 1001;
    VArray *bytebufs    = VA_new(num_strings);

    for (uint32_t i = 0; i < num_strings; ++i) {
        char buf[1201];
        int size = rand() % 1200 + 1;
        for (int i = 0; i < size; ++i) {
            buf[i] = rand();
        }
        ByteBuf *bytebuf = BB_new_bytes(&buf, size);
        VA_Push(bytebufs, (Obj*)bytebuf);
    }

    VA_Sort(bytebufs, NULL, NULL);
    S_test_sort(runner, bytebufs, 15000,
                "Random binary strings of random length");

    DECREF(bytebufs);
}

static void
test_run(TestBatchRunner *runner) {
    VArray *letters = VA_new(26);
    for (int i = 0; i < 26; ++i) {
        char ch = 'a' + i;
        ByteBuf *bytebuf = BB_new_bytes(&ch, 1);
        VA_Push(letters, (Obj*)bytebuf);
    }
    BBSortEx *run = BBSortEx_new(0x1000000, letters);
    BBSortEx_Set_Mem_Thresh(run, 5);

    BBSortEx_Refill(run);
    TEST_INT_EQ(runner, BBSortEx_Buffer_Count(run), 5,
                "Refill doesn't exceed memory threshold");

    Obj *endpost = BBSortEx_Peek_Last(run);
    ByteBuf *wanted = BB_new_bytes("e", 1);
    TEST_TRUE(runner, BB_Equals(wanted, endpost), "Peek_Last");

    VArray *elems = VA_new(26);
    do {
        while (BBSortEx_Buffer_Count(run) > 0) {
            Obj *object = BBSortEx_Fetch(run);
            VA_Push(elems, object);
        }
    } while (BBSortEx_Refill(run) > 0);
    TEST_TRUE(runner, VA_Equals(elems, (Obj*)letters), "retrieve all elems");

    DECREF(elems);
    DECREF(wanted);
    DECREF(endpost);
    DECREF(letters);
    DECREF(run);
}

void
TestSortExternal_Run_IMP(TestSortExternal *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 19);

    srand((unsigned int)time((time_t*)NULL));
    S_init_bytebufs();
    test_bbsortex(runner);
    test_clear_buffer(runner);
    test_sort_letters(runner);
    test_sort_nothing(runner);
    test_sort_packed_ints(runner);
    test_sort_random_strings(runner);
    test_run(runner);
    S_destroy_bytebufs();
}


