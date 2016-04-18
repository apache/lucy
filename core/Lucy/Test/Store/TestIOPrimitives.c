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

#define C_TESTLUCY_TESTINSTREAM
#define C_LUCY_INSTREAM
#define C_LUCY_FILEWINDOW
#include <stdlib.h>
#include <time.h>

#include "charmony.h"

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestUtils.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Store/TestIOPrimitives.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Util/NumberUtils.h"

TestIOPrimitives*
TestIOPrimitives_new() {
    return (TestIOPrimitives*)Class_Make_Obj(TESTIOPRIMITIVES);
}

static void
test_i8(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    int i;

    for (i = -128; i < 128; i++) {
        OutStream_Write_I8(outstream, i);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = -128; i < 128; i++) {
        if (InStream_Read_I8(instream) != i) { break; }
    }
    TEST_INT_EQ(runner, i, 128, "round trip i8 successful for %d out of 256",
                i + 128);

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_u8(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    int i;

    for (i = 0; i < 256; i++) {
        OutStream_Write_U8(outstream, i);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 256; i++) {
        if (InStream_Read_U8(instream) != i) { break; }
    }
    TEST_INT_EQ(runner, i, 256,
                "round trip u8 successful for %d out of 256", i);

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_i32(TestBatchRunner *runner) {
    int64_t    *ints = TestUtils_random_i64s(NULL, 1000, INT32_MIN, INT32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = INT32_MIN;
    ints[1] = INT32_MIN + 1;
    ints[2] = INT32_MAX;
    ints[3] = INT32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_I32(outstream, (int32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int32_t got = InStream_Read_I32(instream);
        if (got != ints[i]) {
            FAIL(runner, "i32 round trip failed: %ld, %ld", (long)got,
                 (long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "i32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_u32(TestBatchRunner *runner) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, UINT32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = UINT32_MAX;
    ints[3] = UINT32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_U32(outstream, (uint32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint32_t got = InStream_Read_U32(instream);
        if (got != ints[i]) {
            FAIL(runner, "u32 round trip failed: %lu, %lu", (unsigned long)got,
                 (unsigned long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "u32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_i64(TestBatchRunner *runner) {
    int64_t    *ints = TestUtils_random_i64s(NULL, 1000, INT64_MIN, INT64_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = INT64_MIN;
    ints[1] = INT64_MIN + 1;
    ints[2] = INT64_MAX;
    ints[3] = INT64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_I64(outstream, ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int64_t got = InStream_Read_I64(instream);
        if (got != ints[i]) {
            FAIL(runner, "i64 round trip failed: %" PRId64 ", %" PRId64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "i64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}


static void
test_u64(TestBatchRunner *runner) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, UINT64_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = UINT64_MAX;
    ints[3] = UINT64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_U64(outstream, ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint64_t got = InStream_Read_U64(instream);
        if (got != ints[i]) {
            FAIL(runner, "u64 round trip failed: %" PRIu64 ", %" PRIu64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "u64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_ci32(TestBatchRunner *runner) {
    int64_t *ints = TestUtils_random_i64s(NULL, 1000, INT32_MIN, INT32_MAX);
    RAMFile *file = RAMFile_new(NULL, false);
    OutStream *outstream = OutStream_open((Obj*)file);
    InStream *instream;
    size_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = -1;
    ints[3] = INT32_MAX;
    ints[4] = INT32_MIN;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_CI32(outstream, (int32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int32_t got = InStream_Read_CI32(instream);
        if ((int64_t)got != ints[i]) {
            FAIL(runner, "ci32 round trip failed: %" PRId32 ", %" PRId64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "ci32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_cu32(TestBatchRunner *runner) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, UINT32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = UINT32_MAX;
    ints[3] = UINT32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_CU32(outstream, (uint32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint32_t got = InStream_Read_CU32(instream);
        if (got != ints[i]) {
            FAIL(runner, "cu32 round trip failed: %lu, %lu", (unsigned long)got,
                 (unsigned long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "cu32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_ci64(TestBatchRunner *runner) {
    int64_t *ints = TestUtils_random_i64s(NULL, 1000, INT64_MIN, INT64_MAX);
    RAMFile *file = RAMFile_new(NULL, false);
    OutStream *outstream = OutStream_open((Obj*)file);
    InStream *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = -1;
    ints[3] = INT64_MAX;
    ints[4] = INT64_MIN;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_CI64(outstream, ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int64_t got = InStream_Read_CI64(instream);
        if (got != ints[i]) {
            FAIL(runner, "ci64 round trip failed: %" PRId64 ", %" PRId64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "ci64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_cu64(TestBatchRunner *runner) {
    uint64_t   *ints   = TestUtils_random_u64s(NULL, 1000, 0, UINT64_MAX);
    RAMFile    *file     = RAMFile_new(NULL, false);
    RAMFile    *raw_file = RAMFile_new(NULL, false);
    OutStream  *outstream     = OutStream_open((Obj*)file);
    OutStream  *raw_outstream = OutStream_open((Obj*)raw_file);
    InStream   *instream;
    InStream   *raw_instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = UINT64_MAX;
    ints[3] = UINT64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_CU64(outstream, ints[i]);
        OutStream_Write_CU64(raw_outstream, ints[i]);
    }
    OutStream_Close(outstream);
    OutStream_Close(raw_outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint64_t got = InStream_Read_CU64(instream);
        if (got != ints[i]) {
            FAIL(runner, "cu64 round trip failed: %" PRIu64 ", %" PRIu64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "cu64 round trip");
    }

    raw_instream = InStream_open((Obj*)raw_file);
    for (i = 0; i < 1000; i++) {
        char  buffer[10];
        const char *buf = buffer;
        size_t size = InStream_Read_Raw_C64(raw_instream, buffer);
        uint64_t got = NumUtil_decode_cu64(&buf);
        UNUSED_VAR(size);
        if (got != ints[i]) {
            FAIL(runner, "Read_Raw_C64 failed: %" PRIu64 ", %" PRIu64,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "Read_Raw_C64");
    }

    DECREF(raw_instream);
    DECREF(instream);
    DECREF(raw_outstream);
    DECREF(outstream);
    DECREF(raw_file);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_f32(TestBatchRunner *runner) {
    double     *f64s   = TestUtils_random_f64s(NULL, 1000);
    float      *values = (float*)MALLOCATE(1000 * sizeof(float));
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Truncate.
    for (i = 0; i < 1000; i++) {
        values[i] = (float)f64s[i];
    }

    // Test boundaries.
    values[0] = 0.0f;
    values[1] = 1.0f;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_F32(outstream, values[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        float got = InStream_Read_F32(instream);
        if (got != values[i]) {
            FAIL(runner, "f32 round trip failed: %f, %f", got, values[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "f32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(values);
    FREEMEM(f64s);
}

static void
test_f64(TestBatchRunner *runner) {
    double     *values = TestUtils_random_f64s(NULL, 1000);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    values[0] = 0.0;
    values[1] = 1.0;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_F64(outstream, values[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        double got = InStream_Read_F64(instream);
        if (got != values[i]) {
            FAIL(runner, "f64 round trip failed: %f, %f", got, values[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(runner, "f64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(values);
}

void
TestIOPrimitives_Run_IMP(TestIOPrimitives *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 13);
    srand((unsigned int)time((time_t*)NULL));
    test_i8(runner);
    test_u8(runner);
    test_i32(runner);
    test_u32(runner);
    test_i64(runner);
    test_u64(runner);
    test_ci32(runner);
    test_cu32(runner);
    test_ci64(runner);
    test_cu64(runner);
    test_f32(runner);
    test_f64(runner);
}


