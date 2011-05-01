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

#define C_LUCY_TESTINSTREAM
#define C_LUCY_INSTREAM
#define C_LUCY_FILEWINDOW
#include <stdlib.h>
#include <time.h>

#include "Lucy/Util/ToolSet.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Store/TestIOPrimitives.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Util/NumberUtils.h"

static void
test_i8(TestBatch *batch) {
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
    TEST_INT_EQ(batch, i, 128, "round trip i8 successful for %d out of 256",
                i + 128);

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_u8(TestBatch *batch) {
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
    TEST_INT_EQ(batch, i, 256,
                "round trip u8 successful for %d out of 256", i);

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_i32(TestBatch *batch) {
    int64_t    *ints = TestUtils_random_i64s(NULL, 1000, I32_MIN, I32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = I32_MIN;
    ints[1] = I32_MIN + 1;
    ints[2] = I32_MAX;
    ints[3] = I32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_I32(outstream, (int32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int32_t got = InStream_Read_I32(instream);
        if (got != ints[i]) {
            FAIL(batch, "i32 round trip failed: %ld, %ld", (long)got,
                 (long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "i32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_u32(TestBatch *batch) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, U32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = U32_MAX;
    ints[3] = U32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_U32(outstream, (uint32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint32_t got = InStream_Read_U32(instream);
        if (got != ints[i]) {
            FAIL(batch, "u32 round trip failed: %lu, %lu", (unsigned long)got,
                 (unsigned long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "u32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_i64(TestBatch *batch) {
    int64_t    *ints = TestUtils_random_i64s(NULL, 1000, I64_MIN, I64_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = I64_MIN;
    ints[1] = I64_MIN + 1;
    ints[2] = I64_MAX;
    ints[3] = I64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_I64(outstream, ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        int64_t got = InStream_Read_I64(instream);
        if (got != ints[i]) {
            FAIL(batch, "i64 round trip failed: %" I64P ", %" I64P,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "i64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}


static void
test_u64(TestBatch *batch) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, U64_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = U64_MAX;
    ints[3] = U64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_U64(outstream, ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint64_t got = InStream_Read_U64(instream);
        if (got != ints[i]) {
            FAIL(batch, "u64 round trip failed: %" U64P ", %" U64P,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "u64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_c32(TestBatch *batch) {
    uint64_t   *ints = TestUtils_random_u64s(NULL, 1000, 0, U32_MAX);
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    uint32_t i;

    // Test boundaries.
    ints[0] = 0;
    ints[1] = 1;
    ints[2] = U32_MAX;
    ints[3] = U32_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_C32(outstream, (uint32_t)ints[i]);
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint32_t got = InStream_Read_C32(instream);
        if (got != ints[i]) {
            FAIL(batch, "c32 round trip failed: %lu, %lu", (unsigned long)got,
                 (unsigned long)ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "c32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(ints);
}

static void
test_c64(TestBatch *batch) {
    uint64_t   *ints   = TestUtils_random_u64s(NULL, 1000, 0, U64_MAX);
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
    ints[2] = U64_MAX;
    ints[3] = U64_MAX - 1;

    for (i = 0; i < 1000; i++) {
        OutStream_Write_C64(outstream, ints[i]);
        OutStream_Write_C64(raw_outstream, ints[i]);
    }
    OutStream_Close(outstream);
    OutStream_Close(raw_outstream);

    instream = InStream_open((Obj*)file);
    for (i = 0; i < 1000; i++) {
        uint64_t got = InStream_Read_C64(instream);
        if (got != ints[i]) {
            FAIL(batch, "c64 round trip failed: %" U64P ", %" U64P,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "c64 round trip");
    }

    raw_instream = InStream_open((Obj*)raw_file);
    for (i = 0; i < 1000; i++) {
        char  buffer[10];
        char *buf = buffer;
        size_t size = InStream_Read_Raw_C64(raw_instream, buffer);
        uint64_t got = NumUtil_decode_c64(&buf);
        UNUSED_VAR(size);
        if (got != ints[i]) {
            FAIL(batch, "Read_Raw_C64 failed: %" U64P ", %" U64P,
                 got, ints[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "Read_Raw_C64");
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
test_f32(TestBatch *batch) {
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
            FAIL(batch, "f32 round trip failed: %f, %f", got, values[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "f32 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(values);
    FREEMEM(f64s);
}

static void
test_f64(TestBatch *batch) {
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
            FAIL(batch, "f64 round trip failed: %f, %f", got, values[i]);
            break;
        }
    }
    if (i == 1000) {
        PASS(batch, "f64 round trip");
    }

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
    FREEMEM(values);
}

void
TestIOPrimitives_run_tests() {
    TestBatch *batch = TestBatch_new(11);

    srand((unsigned int)time((time_t*)NULL));
    TestBatch_Plan(batch);

    test_i8(batch);
    test_u8(batch);
    test_i32(batch);
    test_u32(batch);
    test_i64(batch);
    test_u64(batch);
    test_c32(batch);
    test_c64(batch);
    test_f32(batch);
    test_f64(batch);

    DECREF(batch);
}


