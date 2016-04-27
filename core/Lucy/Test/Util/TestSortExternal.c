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

#include "Clownfish/Blob.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Util/BlobSortEx.h"
#include "Lucy/Util/SortExternal.h"

static Blob *a_blob;
static Blob *b_blob;
static Blob *c_blob;
static Blob *d_blob;
static Blob *x_blob;
static Blob *y_blob;
static Blob *z_blob;

TestSortExternal*
TestSortExternal_new() {
    return (TestSortExternal*)Class_Make_Obj(TESTSORTEXTERNAL);
}

static void
S_init_blobs() {
    a_blob = Blob_new("a", 1);
    b_blob = Blob_new("b", 1);
    c_blob = Blob_new("c", 1);
    d_blob = Blob_new("d", 1);
    x_blob = Blob_new("x", 1);
    y_blob = Blob_new("y", 1);
    z_blob = Blob_new("z", 1);
}

static void
S_destroy_blobs() {
    DECREF(a_blob);
    DECREF(b_blob);
    DECREF(c_blob);
    DECREF(d_blob);
    DECREF(x_blob);
    DECREF(y_blob);
    DECREF(z_blob);
}

static void
test_bbsortex(TestBatchRunner *runner) {
    BlobSortEx *sortex = BlobSortEx_new(4, NULL);

    BlobSortEx_Feed(sortex, INCREF(c_blob));
    TEST_INT_EQ(runner, BlobSortEx_Buffer_Count(sortex), 1,
                "feed elem into cache");

    BlobSortEx_Feed(sortex, INCREF(b_blob));
    BlobSortEx_Feed(sortex, INCREF(d_blob));
    BlobSortEx_Sort_Buffer(sortex);

    {
        Vector *cache  = BlobSortEx_Peek_Cache(sortex);
        Vector *wanted = Vec_new(3);
        Vec_Push(wanted, INCREF(b_blob));
        Vec_Push(wanted, INCREF(c_blob));
        Vec_Push(wanted, INCREF(d_blob));
        TEST_TRUE(runner, Vec_Equals(cache, (Obj*)wanted), "sort cache");
        DECREF(wanted);
        DECREF(cache);
    }

    BlobSortEx_Feed(sortex, INCREF(a_blob));
    TEST_INT_EQ(runner, BlobSortEx_Buffer_Count(sortex), 0,
                "cache flushed automatically when mem_thresh crossed");
    TEST_INT_EQ(runner, BlobSortEx_Get_Num_Runs(sortex), 1, "run added");

    Vector *external = Vec_new(3);
    Vec_Push(external, INCREF(x_blob));
    Vec_Push(external, INCREF(y_blob));
    Vec_Push(external, INCREF(z_blob));
    BlobSortEx *run = BlobSortEx_new(0x1000000, external);
    BlobSortEx_Add_Run(sortex, (SortExternal*)run);
    BlobSortEx_Flip(sortex);

    {
        Vector *got = Vec_new(7);
        Obj *object;
        while (NULL != (object = BlobSortEx_Fetch(sortex))) {
            Vec_Push(got, object);
        }

        Vector *wanted = Vec_new(7);
        Vec_Push(wanted, INCREF(a_blob));
        Vec_Push(wanted, INCREF(b_blob));
        Vec_Push(wanted, INCREF(c_blob));
        Vec_Push(wanted, INCREF(d_blob));
        Vec_Push(wanted, INCREF(x_blob));
        Vec_Push(wanted, INCREF(y_blob));
        Vec_Push(wanted, INCREF(z_blob));

        TEST_TRUE(runner, Vec_Equals(got, (Obj*)wanted), "Add_Run");

        DECREF(wanted);
        DECREF(got);
    }

    DECREF(external);
    DECREF(sortex);
}

static void
test_clear_buffer(TestBatchRunner *runner) {
    BlobSortEx *sortex = BlobSortEx_new(4, NULL);

    BlobSortEx_Feed(sortex, INCREF(c_blob));
    BlobSortEx_Clear_Buffer(sortex);
    TEST_INT_EQ(runner, BlobSortEx_Buffer_Count(sortex), 0, "Clear_Buffer");

    BlobSortEx_Feed(sortex, INCREF(b_blob));
    BlobSortEx_Feed(sortex, INCREF(a_blob));
    BlobSortEx_Flush(sortex);
    BlobSortEx_Flip(sortex);
    Obj *object = BlobSortEx_Peek(sortex);
    TEST_TRUE(runner, Blob_Equals(a_blob, object), "Peek");

    Vector *got = Vec_new(2);
    while (NULL != (object = BlobSortEx_Fetch(sortex))) {
        Vec_Push(got, object);
    }
    Vector *wanted = Vec_new(2);
    Vec_Push(wanted, INCREF(a_blob));
    Vec_Push(wanted, INCREF(b_blob));
    TEST_TRUE(runner, Vec_Equals(got, (Obj*)wanted),
              "elements cleared via Clear_Buffer truly cleared");

    DECREF(wanted);
    DECREF(got);
    DECREF(sortex);
}

static void
S_test_sort(TestBatchRunner *runner, Vector *blobs, uint32_t mem_thresh,
            const char *test_name) {
    size_t       size     = Vec_Get_Size(blobs);
    BlobSortEx  *sortex   = BlobSortEx_new(mem_thresh, NULL);
    Blob       **shuffled = (Blob**)MALLOCATE(size * sizeof(Blob*));

    for (size_t i = 0; i < size; ++i) {
        shuffled[i] = (Blob*)CERTIFY(Vec_Fetch(blobs, i), BLOB);
    }
    for (int i = (int)size - 1; i > 0; --i) {
        int shuffle_pos = rand() % (i + 1);
        Blob *temp = shuffled[shuffle_pos];
        shuffled[shuffle_pos] = shuffled[i];
        shuffled[i] = temp;
    }
    for (size_t i = 0; i < size; ++i) {
        BlobSortEx_Feed(sortex, INCREF(shuffled[i]));
    }

    BlobSortEx_Flip(sortex);
    Vector *got = Vec_new(size);
    Obj *object;
    while (NULL != (object = BlobSortEx_Fetch(sortex))) {
        Vec_Push(got, object);
    }
    TEST_TRUE(runner, Vec_Equals(got, (Obj*)blobs), test_name);

    FREEMEM(shuffled);
    DECREF(got);
    DECREF(sortex);
}

static void
S_test_sort_letters(TestBatchRunner *runner, const char *letters,
                    uint32_t mem_thresh, const char *test_name) {
    size_t  num_letters = strlen(letters);
    Vector *blobs       = Vec_new(num_letters);

    for (size_t i = 0; i < num_letters; ++i) {
        char ch = letters[i];
        size_t size = ch == '_' ? 0 : 1;
        Blob *blob = Blob_new(&ch, size);
        Vec_Push(blobs, (Obj*)blob);
    }

    S_test_sort(runner, blobs, mem_thresh, test_name);

    DECREF(blobs);
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
    BlobSortEx *sortex = BlobSortEx_new(0x1000000, NULL);
    BlobSortEx_Flip(sortex);
    TEST_TRUE(runner, BlobSortEx_Fetch(sortex) == NULL,
              "Sorting nothing returns undef");
    DECREF(sortex);
}

static void
test_sort_packed_ints(TestBatchRunner *runner) {
    size_t  num_ints = 11001;
    Vector *blobs    = Vec_new(num_ints);

    for (uint32_t i = 0; i < num_ints; ++i) {
        char buf[4];
        buf[0] = i >> 24;
        buf[1] = i >> 16;
        buf[2] = i >> 8;
        buf[3] = i;
        Blob *blob = Blob_new(buf, 4);
        Vec_Push(blobs, (Obj*)blob);
    }

    S_test_sort(runner, blobs, 5000, "Sorting packed integers...");

    DECREF(blobs);
}

static void
test_sort_random_strings(TestBatchRunner *runner) {
    size_t  num_strings = 1001;
    Vector *blobs       = Vec_new(num_strings);

    for (uint32_t i = 0; i < num_strings; ++i) {
        char buf[1201];
        int size = rand() % 1200 + 1;
        for (int i = 0; i < size; ++i) {
            buf[i] = rand();
        }
        Blob *blob = Blob_new(buf, (size_t)size);
        Vec_Push(blobs, (Obj*)blob);
    }

    Vec_Sort(blobs);
    S_test_sort(runner, blobs, 15000,
                "Random binary strings of random length");

    DECREF(blobs);
}

static void
test_run(TestBatchRunner *runner) {
    Vector *letters = Vec_new(26);
    for (int i = 0; i < 26; ++i) {
        char ch = 'a' + i;
        Blob *blob = Blob_new(&ch, 1);
        Vec_Push(letters, (Obj*)blob);
    }
    BlobSortEx *run = BlobSortEx_new(0x1000000, letters);
    BlobSortEx_Set_Mem_Thresh(run, 5);

    BlobSortEx_Refill(run);
    TEST_INT_EQ(runner, BlobSortEx_Buffer_Count(run), 5,
                "Refill doesn't exceed memory threshold");

    Obj *endpost = BlobSortEx_Peek_Last(run);
    Blob *wanted = Blob_new("e", 1);
    TEST_TRUE(runner, Blob_Equals(wanted, endpost), "Peek_Last");

    Vector *elems = Vec_new(26);
    do {
        while (BlobSortEx_Buffer_Count(run) > 0) {
            Obj *object = BlobSortEx_Fetch(run);
            Vec_Push(elems, object);
        }
    } while (BlobSortEx_Refill(run) > 0);
    TEST_TRUE(runner, Vec_Equals(elems, (Obj*)letters), "retrieve all elems");

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
    S_init_blobs();
    test_bbsortex(runner);
    test_clear_buffer(runner);
    test_sort_letters(runner);
    test_sort_nothing(runner);
    test_sort_packed_ints(runner);
    test_sort_random_strings(runner);
    test_run(runner);
    S_destroy_blobs();
}


