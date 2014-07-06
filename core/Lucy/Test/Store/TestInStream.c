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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Store/TestInStream.h"
#include "Lucy/Test/Store/MockFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Clownfish/Util/NumberUtils.h"

TestInStream*
TestInStream_new() {
    return (TestInStream*)Class_Make_Obj(TESTINSTREAM);
}

static void
test_refill(TestBatchRunner *runner) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    char        scratch[5];
    InStreamIVARS *ivars;

    for (int32_t i = 0; i < 1023; i++) {
        OutStream_Write_U8(outstream, 'x');
    }
    OutStream_Write_U8(outstream, 'y');
    OutStream_Write_U8(outstream, 'z');
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    ivars = InStream_IVARS(instream);
    InStream_Refill(instream);
    TEST_INT_EQ(runner, ivars->limit - ivars->buf, IO_STREAM_BUF_SIZE,
                "Refill");
    TEST_INT_EQ(runner, (long)InStream_Tell(instream), 0,
                "Correct file pos after standing-start Refill()");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    ivars = InStream_IVARS(instream);
    InStream_Fill(instream, 30);
    TEST_INT_EQ(runner, ivars->limit - ivars->buf, 30, "Fill()");
    TEST_INT_EQ(runner, (long)InStream_Tell(instream), 0,
                "Correct file pos after standing-start Fill()");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    ivars = InStream_IVARS(instream);
    InStream_Read_Bytes(instream, scratch, 5);
    TEST_INT_EQ(runner, ivars->limit - ivars->buf,
                IO_STREAM_BUF_SIZE - 5, "small read triggers refill");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    ivars = InStream_IVARS(instream);
    TEST_INT_EQ(runner, InStream_Read_U8(instream), 'x', "Read_U8");
    InStream_Seek(instream, 1023);
    TEST_INT_EQ(runner, (long)FileWindow_IVARS(ivars->window)->offset, 0,
                "no unnecessary refill on Seek");
    TEST_INT_EQ(runner, (long)InStream_Tell(instream), 1023, "Seek/Tell");
    TEST_INT_EQ(runner, InStream_Read_U8(instream), 'y',
                "correct data after in-buffer Seek()");
    TEST_INT_EQ(runner, InStream_Read_U8(instream), 'z', "automatic Refill");
    TEST_TRUE(runner, (FileWindow_IVARS(ivars->window)->offset != 0),
              "refilled");

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_Clone_and_Reopen(TestBatchRunner *runner) {
    StackString *foo       = SSTR_WRAP_UTF8("foo", 3);
    StackString *bar       = SSTR_WRAP_UTF8("bar", 3);
    RAMFile       *file      = RAMFile_new(NULL, false);
    OutStream     *outstream = OutStream_open((Obj*)file);
    RAMFileHandle *fh;
    InStream      *instream;
    InStream      *clone;
    InStream      *reopened;

    for (uint32_t i = 0; i < 26; i++) {
        OutStream_Write_U8(outstream, 'a' + i);
    }
    OutStream_Close(outstream);

    fh = RAMFH_open((String*)foo, FH_READ_ONLY, file);
    instream = InStream_open((Obj*)fh);
    InStream_Seek(instream, 1);
    TEST_TRUE(runner, Str_Equals(InStream_Get_Filename(instream), (Obj*)foo),
              "Get_Filename");

    clone    = InStream_Clone(instream);
    TEST_TRUE(runner, Str_Equals(InStream_Get_Filename(clone), (Obj*)foo),
              "Clones have same filename");
    TEST_TRUE(runner, InStream_Length(instream) == InStream_Length(clone),
              "Clones have same length");
    TEST_TRUE(runner, InStream_Read_U8(instream) == InStream_Read_U8(clone),
              "Clones start at same file position");

    reopened = InStream_Reopen(instream, (String*)bar, 25, 1);
    TEST_TRUE(runner, Str_Equals(InStream_Get_Filename(reopened), (Obj*)bar),
              "Reopened InStreams take new filename");
    TEST_TRUE(runner, InStream_Read_U8(reopened) == 'z',
              "Reopened stream starts at supplied offset");
    TEST_TRUE(runner, InStream_Length(reopened) == 1,
              "Reopened stream uses supplied length");
    TEST_TRUE(runner, InStream_Tell(reopened) == 1,
              "Tell() uses supplied offset for reopened stream");
    InStream_Seek(reopened, 0);
    TEST_TRUE(runner, InStream_Read_U8(reopened) == 'z',
              "Seek() uses supplied offset for reopened stream");

    DECREF(reopened);
    DECREF(clone);
    DECREF(instream);
    DECREF(outstream);
    DECREF(fh);
    DECREF(file);
}

static void
test_Close(TestBatchRunner *runner) {
    RAMFile  *file     = RAMFile_new(NULL, false);
    InStream *instream = InStream_open((Obj*)file);
    InStream_Close(instream);
    TEST_TRUE(runner, InStream_IVARS(instream)->file_handle == NULL,
              "Close decrements FileHandle's refcount");
    DECREF(instream);
    DECREF(file);
}

static void
test_Seek_and_Tell(TestBatchRunner *runner) {
    int64_t     gb1      = INT64_C(0x40000000);
    int64_t     gb3      = gb1 * 3;
    int64_t     gb6      = gb1 * 6;
    int64_t     gb12     = gb1 * 12;
    FileHandle *fh       = (FileHandle*)MockFileHandle_new(NULL, gb12);
    InStream   *instream = InStream_open((Obj*)fh);
    InStreamIVARS *const ivars = InStream_IVARS(instream);

    InStream_Buf(instream, 10000);
    TEST_TRUE(runner, ivars->limit == ((char*)NULL) + 10000,
              "InStream_Buf sets limit");

    InStream_Seek(instream, gb6);
    TEST_TRUE(runner, InStream_Tell(instream) == gb6,
              "Tell after seek forwards outside buffer");
    TEST_TRUE(runner, ivars->buf == NULL,
              "Seek forwards outside buffer sets buf to NULL");
    TEST_TRUE(runner, ivars->limit == NULL,
              "Seek forwards outside buffer sets limit to NULL");
    TEST_TRUE(runner, FileWindow_IVARS(ivars->window)->offset == gb6,
              "Seek forwards outside buffer tracks pos in window offset");

    InStream_Buf(instream, (size_t)gb1);
    TEST_TRUE(runner, ivars->limit == ((char*)NULL) + gb1,
              "InStream_Buf sets limit");

    InStream_Seek(instream, gb6 + 10);
    TEST_TRUE(runner, InStream_Tell(instream) == gb6 + 10,
              "Tell after seek forwards within buffer");
    TEST_TRUE(runner, ivars->buf == ((char*)NULL) + 10,
              "Seek within buffer sets buf");
    TEST_TRUE(runner, ivars->limit == ((char*)NULL) + gb1,
              "Seek within buffer leaves limit alone");

    InStream_Seek(instream, gb6 + 1);
    TEST_TRUE(runner, InStream_Tell(instream) == gb6 + 1,
              "Tell after seek backwards within buffer");
    TEST_TRUE(runner, ivars->buf == ((char*)NULL) + 1,
              "Seek backwards within buffer sets buf");
    TEST_TRUE(runner, ivars->limit == ((char*)NULL) + gb1,
              "Seek backwards within buffer leaves limit alone");

    InStream_Seek(instream, gb3);
    TEST_TRUE(runner, InStream_Tell(instream) == gb3,
              "Tell after seek backwards outside buffer");
    TEST_TRUE(runner, ivars->buf == NULL,
              "Seek backwards outside buffer sets buf to NULL");
    TEST_TRUE(runner, ivars->limit == NULL,
              "Seek backwards outside buffer sets limit to NULL");
    TEST_TRUE(runner, FileWindow_IVARS(ivars->window)->offset == gb3,
              "Seek backwards outside buffer tracks pos in window offset");

    DECREF(instream);
    DECREF(fh);
}

void
TestInStream_Run_IMP(TestInStream *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 37);
    test_refill(runner);
    test_Clone_and_Reopen(runner);
    test_Close(runner);
    test_Seek_and_Tell(runner);
}


