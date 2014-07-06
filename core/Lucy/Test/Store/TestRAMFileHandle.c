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

#define C_TESTLUCY_TESTINSTREAM
#define C_LUCY_INSTREAM
#define C_LUCY_FILEWINDOW
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestRAMFileHandle.h"
#include "Lucy/Store/RAMFileHandle.h"
#include "Lucy/Store/FileWindow.h"
#include "Lucy/Store/RAMFile.h"

TestRAMFileHandle*
TestRAMFH_new() {
    return (TestRAMFileHandle*)Class_Make_Obj(TESTRAMFILEHANDLE);
}

static void
test_open(TestBatchRunner *runner) {
    RAMFileHandle *fh;

    Err_set_error(NULL);
    fh = RAMFH_open(NULL, FH_WRITE_ONLY, NULL);
    TEST_TRUE(runner, fh == NULL,
              "open() without a RAMFile or FH_CREATE returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "open() without a RAMFile or FH_CREATE sets error");
}

static void
test_Read_Write(TestBatchRunner *runner) {
    RAMFile *file = RAMFile_new(NULL, false);
    RAMFileHandle *fh = RAMFH_open(NULL, FH_WRITE_ONLY, file);
    const char *foo = "foo";
    const char *bar = "bar";
    char buffer[12];
    char *buf = buffer;

    TEST_TRUE(runner, Str_Equals_Utf8(RAMFH_Get_Path(fh), "", 0),
              "NULL arg as filepath yields empty string");

    TEST_TRUE(runner, RAMFH_Write(fh, foo, 3), "Write returns success");
    TEST_TRUE(runner, RAMFH_Length(fh) == 3, "Length after one Write");
    TEST_TRUE(runner, RAMFH_Write(fh, bar, 3), "Write returns success");
    TEST_TRUE(runner, RAMFH_Length(fh) == 6, "Length after two Writes");

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Read(fh, buf, 0, 2),
               "Reading from a write-only handle returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Reading from a write-only handle sets error");

    // Reopen for reading.
    DECREF(fh);
    fh = RAMFH_open(NULL, FH_READ_ONLY, file);
    TEST_TRUE(runner, RAMFile_Read_Only(file),
              "FH_READ_ONLY propagates to RAMFile's read_only property");

    TEST_TRUE(runner, RAMFH_Read(fh, buf, 0, 6), "Read returns success");
    TEST_TRUE(runner, strncmp(buf, "foobar", 6) == 0, "Read/Write");
    TEST_TRUE(runner, RAMFH_Read(fh, buf, 2, 3), "Read returns success");
    TEST_TRUE(runner, strncmp(buf, "oba", 3) == 0, "Read with offset");

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Read(fh, buf, -1, 4),
               "Read() with a negative offset returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Read() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Read(fh, buf, 6, 1),
               "Read() past EOF returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Read() past EOF sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Write(fh, foo, 3),
               "Writing to a read-only handle returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Writing to a read-only handle sets error");

    DECREF(fh);
    DECREF(file);
}

static void
test_Grow_and_Get_File(TestBatchRunner *runner) {
    RAMFileHandle *fh = RAMFH_open(NULL, FH_WRITE_ONLY | FH_CREATE, NULL);
    RAMFile *ram_file = RAMFH_Get_File(fh);
    ByteBuf *contents = RAMFile_Get_Contents(ram_file);

    RAMFH_Grow(fh, 100);
    TEST_TRUE(runner, BB_Get_Capacity(contents) >= 100, "Grow");

    DECREF(fh);
}

static void
test_Close(TestBatchRunner *runner) {
    RAMFileHandle *fh = RAMFH_open(NULL, FH_WRITE_ONLY | FH_CREATE, NULL);
    TEST_TRUE(runner, RAMFH_Close(fh), "Close returns true");
    DECREF(fh);
}

static void
test_Window(TestBatchRunner *runner) {
    RAMFile *file = RAMFile_new(NULL, false);
    RAMFileHandle *fh = RAMFH_open(NULL, FH_WRITE_ONLY, file);
    FileWindow *window = FileWindow_new();
    FileWindowIVARS *const window_ivars = FileWindow_IVARS(window);

    for (uint32_t i = 0; i < 1024; i++) {
        RAMFH_Write(fh, "foo ", 4);
    }
    RAMFH_Close(fh);

    // Reopen for reading.
    DECREF(fh);
    fh = RAMFH_open(NULL, FH_READ_ONLY, file);

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Window(fh, window, -1, 4),
               "Window() with a negative offset returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Window() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFH_Window(fh, window, 4000, 1000),
               "Window() past EOF returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Window() past EOF sets error");

    TEST_TRUE(runner, RAMFH_Window(fh, window, 1021, 2),
              "Window() returns true");
    TEST_TRUE(runner, strncmp(window_ivars->buf, "oo", 2) == 0, "Window()");

    TEST_TRUE(runner, RAMFH_Release_Window(fh, window),
              "Release_Window() returns true");
    TEST_TRUE(runner, window_ivars->buf == NULL, "Release_Window() resets buf");
    TEST_TRUE(runner, window_ivars->offset == 0, "Release_Window() resets offset");
    TEST_TRUE(runner, window_ivars->len == 0, "Release_Window() resets len");

    DECREF(window);
    DECREF(fh);
    DECREF(file);
}

void
TestRAMFH_Run_IMP(TestRAMFileHandle *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 32);
    test_open(runner);
    test_Read_Write(runner);
    test_Grow_and_Get_File(runner);
    test_Close(runner);
    test_Window(runner);
}


