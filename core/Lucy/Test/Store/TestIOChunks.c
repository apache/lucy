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
#include "Lucy/Test/Store/TestIOChunks.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Clownfish/Util/NumberUtils.h"

TestIOChunks*
TestIOChunks_new() {
    return (TestIOChunks*)Class_Make_Obj(TESTIOCHUNKS);
}

static void
test_Align(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);

    for (int32_t i = 1; i < 32; i++) {
        int64_t random_bytes = TestUtils_random_u64() % 32;
        while (random_bytes--) { OutStream_Write_U8(outstream, 0); }
        TEST_TRUE(runner, (OutStream_Align(outstream, i) % i) == 0,
                  "Align to %ld", (long)i);
    }
    DECREF(file);
    DECREF(outstream);
}

static void
test_Read_Write_Bytes(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    char        buf[4];

    OutStream_Write_Bytes(outstream, "foo", 4);
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    InStream_Read_Bytes(instream, buf, 4);
    TEST_TRUE(runner, strcmp(buf, "foo") == 0, "Read_Bytes Write_Bytes");

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_Buf(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    size_t      size      = IO_STREAM_BUF_SIZE * 2 + 5;
    InStream   *instream;
    const char *buf;

    for (uint32_t i = 0; i < size; i++) {
        OutStream_Write_U8(outstream, 'a');
    }
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    InStreamIVARS *const ivars = InStream_IVARS(instream);
    buf = InStream_Buf(instream, 5);
    TEST_INT_EQ(runner, ivars->limit - buf, IO_STREAM_BUF_SIZE,
                "Small request bumped up");

    buf += IO_STREAM_BUF_SIZE - 10; // 10 bytes left in buffer.
    InStream_Advance_Buf(instream, buf);

    buf = InStream_Buf(instream, 10);
    TEST_INT_EQ(runner, ivars->limit - buf, 10,
                "Exact request doesn't trigger refill");

    buf = InStream_Buf(instream, 11);
    TEST_INT_EQ(runner, ivars->limit - buf, IO_STREAM_BUF_SIZE,
                "Requesting over limit triggers refill");

    int64_t     expected = InStream_Length(instream) - InStream_Tell(instream);
    const char *buff     = InStream_Buf(instream, 100000);
    int64_t     got      = CHY_PTR_TO_I64(ivars->limit) - CHY_PTR_TO_I64(buff);
    TEST_TRUE(runner, got == expected,
              "Requests greater than file size get pared down");

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

void
TestIOChunks_Run_IMP(TestIOChunks *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 36);
    srand((unsigned int)time((time_t*)NULL));
    test_Align(runner);
    test_Read_Write_Bytes(runner);
    test_Buf(runner);
}


