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

#include "Lucy/Util/ToolSet.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Store/TestInStream.h"
#include "Lucy/Test/Store/MockFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Store/RAMFile.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Util/NumberUtils.h"

static void
test_refill(TestBatch *batch) {
    RAMFile    *file      = RAMFile_new(NULL, false);
    OutStream  *outstream = OutStream_open((Obj*)file);
    InStream   *instream;
    char        scratch[5];
    int32_t i;

    for (i = 0; i < 1023; i++) {
        OutStream_Write_U8(outstream, 'x');
    }
    OutStream_Write_U8(outstream, 'y');
    OutStream_Write_U8(outstream, 'z');
    OutStream_Close(outstream);

    instream = InStream_open((Obj*)file);
    InStream_Refill(instream);
    TEST_INT_EQ(batch, instream->limit - instream->buf, IO_STREAM_BUF_SIZE,
                "Refill");
    TEST_INT_EQ(batch, (long)InStream_Tell(instream), 0,
                "Correct file pos after standing-start Refill()");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    InStream_Fill(instream, 30);
    TEST_INT_EQ(batch, instream->limit - instream->buf, 30, "Fill()");
    TEST_INT_EQ(batch, (long)InStream_Tell(instream), 0,
                "Correct file pos after standing-start Fill()");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    InStream_Read_Bytes(instream, scratch, 5);
    TEST_INT_EQ(batch, instream->limit - instream->buf,
                IO_STREAM_BUF_SIZE - 5, "small read triggers refill");
    DECREF(instream);

    instream = InStream_open((Obj*)file);
    TEST_INT_EQ(batch, InStream_Read_U8(instream), 'x', "Read_U8");
    InStream_Seek(instream, 1023);
    TEST_INT_EQ(batch, (long)instream->window->offset, 0,
                "no unnecessary refill on Seek");
    TEST_INT_EQ(batch, (long)InStream_Tell(instream), 1023, "Seek/Tell");
    TEST_INT_EQ(batch, InStream_Read_U8(instream), 'y',
                "correct data after in-buffer Seek()");
    TEST_INT_EQ(batch, InStream_Read_U8(instream), 'z', "automatic Refill");
    TEST_TRUE(batch, (instream->window->offset != 0), "refilled");

    DECREF(instream);
    DECREF(outstream);
    DECREF(file);
}

static void
test_Clone_and_Reopen(TestBatch *batch) {
    ZombieCharBuf *foo       = ZCB_WRAP_STR("foo", 3);
    ZombieCharBuf *bar       = ZCB_WRAP_STR("bar", 3);
    RAMFile       *file      = RAMFile_new(NULL, false);
    OutStream     *outstream = OutStream_open((Obj*)file);
    RAMFileHandle *fh;
    InStream      *instream;
    InStream      *clone;
    InStream      *reopened;
    uint32_t i;

    for (i = 0; i < 26; i++) {
        OutStream_Write_U8(outstream, 'a' + i);
    }
    OutStream_Close(outstream);

    fh = RAMFH_open((CharBuf*)foo, FH_READ_ONLY, file);
    instream = InStream_open((Obj*)fh);
    InStream_Seek(instream, 1);
    TEST_TRUE(batch, CB_Equals(InStream_Get_Filename(instream), (Obj*)foo),
              "Get_Filename");

    clone    = InStream_Clone(instream);
    TEST_TRUE(batch, CB_Equals(InStream_Get_Filename(clone), (Obj*)foo),
              "Clones have same filename");
    TEST_TRUE(batch, InStream_Length(instream) == InStream_Length(clone),
              "Clones have same length");
    TEST_TRUE(batch, InStream_Read_U8(instream) == InStream_Read_U8(clone),
              "Clones start at same file position");

    reopened = InStream_Reopen(instream, (CharBuf*)bar, 25, 1);
    TEST_TRUE(batch, CB_Equals(InStream_Get_Filename(reopened), (Obj*)bar),
              "Reopened InStreams take new filename");
    TEST_TRUE(batch, InStream_Read_U8(reopened) == 'z',
              "Reopened stream starts at supplied offset");
    TEST_TRUE(batch, InStream_Length(reopened) == 1,
              "Reopened stream uses supplied length");
    TEST_TRUE(batch, InStream_Tell(reopened) == 1,
              "Tell() uses supplied offset for reopened stream");
    InStream_Seek(reopened, 0);
    TEST_TRUE(batch, InStream_Read_U8(reopened) == 'z',
              "Seek() uses supplied offset for reopened stream");

    DECREF(reopened);
    DECREF(clone);
    DECREF(instream);
    DECREF(outstream);
    DECREF(fh);
    DECREF(file);
}

static void
test_Close(TestBatch *batch) {
    RAMFile  *file     = RAMFile_new(NULL, false);
    InStream *instream = InStream_open((Obj*)file);
    InStream_Close(instream);
    TEST_TRUE(batch, instream->file_handle == NULL,
              "Close decrements FileHandle's refcount");
    DECREF(instream);
    DECREF(file);
}

static void
test_Seek_and_Tell(TestBatch *batch) {
    int64_t     gb1      = I64_C(0x40000000);
    int64_t     gb3      = gb1 * 3;
    int64_t     gb6      = gb1 * 6;
    int64_t     gb12     = gb1 * 12;
    FileHandle *fh       = (FileHandle*)MockFileHandle_new(NULL, gb12);
    InStream   *instream = InStream_open((Obj*)fh);

    InStream_Buf(instream, 10000);
    TEST_TRUE(batch, instream->limit == ((char*)NULL) + 10000,
              "InStream_Buf sets limit");

    InStream_Seek(instream, gb6);
    TEST_TRUE(batch, InStream_Tell(instream) == gb6,
              "Tell after seek forwards outside buffer");
    TEST_TRUE(batch, instream->buf == NULL,
              "Seek forwards outside buffer sets buf to NULL");
    TEST_TRUE(batch, instream->limit == NULL,
              "Seek forwards outside buffer sets limit to NULL");
    TEST_TRUE(batch, instream->window->offset == gb6,
              "Seek forwards outside buffer tracks pos in window offset");

    InStream_Buf(instream, (size_t)gb1);
    TEST_TRUE(batch, instream->limit == ((char*)NULL) + gb1,
              "InStream_Buf sets limit");

    InStream_Seek(instream, gb6 + 10);
    TEST_TRUE(batch, InStream_Tell(instream) == gb6 + 10,
              "Tell after seek forwards within buffer");
    TEST_TRUE(batch, instream->buf == ((char*)NULL) + 10,
              "Seek within buffer sets buf");
    TEST_TRUE(batch, instream->limit == ((char*)NULL) + gb1,
              "Seek within buffer leaves limit alone");

    InStream_Seek(instream, gb6 + 1);
    TEST_TRUE(batch, InStream_Tell(instream) == gb6 + 1,
              "Tell after seek backwards within buffer");
    TEST_TRUE(batch, instream->buf == ((char*)NULL) + 1,
              "Seek backwards within buffer sets buf");
    TEST_TRUE(batch, instream->limit == ((char*)NULL) + gb1,
              "Seek backwards within buffer leaves limit alone");

    InStream_Seek(instream, gb3);
    TEST_TRUE(batch, InStream_Tell(instream) == gb3,
              "Tell after seek backwards outside buffer");
    TEST_TRUE(batch, instream->buf == NULL,
              "Seek backwards outside buffer sets buf to NULL");
    TEST_TRUE(batch, instream->limit == NULL,
              "Seek backwards outside buffer sets limit to NULL");
    TEST_TRUE(batch, instream->window->offset == gb3,
              "Seek backwards outside buffer tracks pos in window offset");

    DECREF(instream);
    DECREF(fh);
}

void
TestInStream_run_tests() {
    TestBatch *batch = TestBatch_new(37);

    TestBatch_Plan(batch);

    test_refill(batch);
    test_Clone_and_Reopen(batch);
    test_Close(batch);
    test_Seek_and_Tell(batch);

    DECREF(batch);
}


