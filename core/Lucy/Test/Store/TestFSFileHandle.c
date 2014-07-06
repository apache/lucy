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

#include <stdio.h> // for remove()

#define C_LUCY_FSFILEHANDLE
#define C_LUCY_FILEWINDOW
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h> // close
#elif defined(CHY_HAS_IO_H)
  #include <io.h> // close
#endif

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSFileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"

TestFSFileHandle*
TestFSFH_new() {
    return (TestFSFileHandle*)Class_Make_Obj(TESTFSFILEHANDLE);
}

static void
test_open(TestBatchRunner *runner) {

    FSFileHandle *fh;
    String *test_filename = (String*)SSTR_WRAP_UTF8("_fstest", 7);

    remove(Str_Get_Ptr8(test_filename));

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "open() with FH_READ_ONLY on non-existent file returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "open() with FH_READ_ONLY on non-existent file sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_WRITE_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "open() without FH_CREATE returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "open() without FH_CREATE sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE);
    TEST_TRUE(runner, fh == NULL,
              "open() without FH_WRITE_ONLY returns NULL");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "open() without FH_WRITE_ONLY sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, fh && FSFH_Is_A(fh, FSFILEHANDLE), "open() succeeds");
    TEST_TRUE(runner, Err_get_error() == NULL, "open() no errors");
    FSFH_Write(fh, "foo", 3);
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, fh == NULL, "FH_EXCLUSIVE blocks open()");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "FH_EXCLUSIVE blocks open(), sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FSFH_Is_A(fh, FSFILEHANDLE),
              "open() for append");
    TEST_TRUE(runner, Err_get_error() == NULL,
              "open() for append -- no errors");
    FSFH_Write(fh, "bar", 3);
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(runner, fh && FSFH_Is_A(fh, FSFILEHANDLE), "open() read only");
    TEST_TRUE(runner, Err_get_error() == NULL,
              "open() read only -- no errors");
    DECREF(fh);

    remove(Str_Get_Ptr8(test_filename));
}

static void
test_Read_Write(TestBatchRunner *runner) {
    FSFileHandle *fh;
    const char *foo = "foo";
    const char *bar = "bar";
    char buffer[12];
    char *buf = buffer;
    String *test_filename = (String*)SSTR_WRAP_UTF8("_fstest", 7);

    remove(Str_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);

    TEST_TRUE(runner, FSFH_Length(fh) == INT64_C(0), "Length initially 0");
    TEST_TRUE(runner, FSFH_Write(fh, foo, 3), "Write returns success");
    TEST_TRUE(runner, FSFH_Length(fh) == INT64_C(3), "Length after Write");
    TEST_TRUE(runner, FSFH_Write(fh, bar, 3), "Write returns success");
    TEST_TRUE(runner, FSFH_Length(fh) == INT64_C(6), "Length after 2 Writes");

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Read(fh, buf, 0, 2),
               "Reading from a write-only handle returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Reading from a write-only handle sets error");
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    // Reopen for reading.
    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);

    TEST_TRUE(runner, FSFH_Length(fh) == INT64_C(6), "Length on Read");
    TEST_TRUE(runner, FSFH_Read(fh, buf, 0, 6), "Read returns success");
    TEST_TRUE(runner, strncmp(buf, "foobar", 6) == 0, "Read/Write");
    TEST_TRUE(runner, FSFH_Read(fh, buf, 2, 3), "Read returns success");
    TEST_TRUE(runner, strncmp(buf, "oba", 3) == 0, "Read with offset");

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Read(fh, buf, -1, 4),
               "Read() with a negative offset returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Read() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Read(fh, buf, 6, 1),
               "Read() past EOF returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Read() past EOF sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Write(fh, foo, 3),
               "Writing to a read-only handle returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Writing to a read-only handle sets error");

    DECREF(fh);
    remove(Str_Get_Ptr8(test_filename));
}

static void
test_Close(TestBatchRunner *runner) {
    String *test_filename = (String*)SSTR_WRAP_UTF8("_fstest", 7);
    FSFileHandle *fh;

    remove(Str_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, FSFH_Close(fh), "Close returns true for write-only");
    DECREF(fh);

    // Simulate an OS error when closing the file descriptor.  This
    // approximates what would happen if, say, we run out of disk space.
    remove(Str_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
#ifdef _MSC_VER
    SKIP(runner, "LUCY-155");
    SKIP(runner, "LUCY-155");
#else
    int saved_fd = FSFH_IVARS(fh)->fd;
    FSFH_IVARS(fh)->fd = -1;
    Err_set_error(NULL);
    bool result = FSFH_Close(fh);
    TEST_FALSE(runner, result, "Failed Close() returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed Close() sets Err_error");
    FSFH_IVARS(fh)->fd = saved_fd;
#endif /* _MSC_VER */
    DECREF(fh);

    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(runner, FSFH_Close(fh), "Close returns true for read-only");

    DECREF(fh);
    remove(Str_Get_Ptr8(test_filename));
}

static void
test_Window(TestBatchRunner *runner) {
    String *test_filename = (String*)SSTR_WRAP_UTF8("_fstest", 7);
    FSFileHandle *fh;
    FileWindow *window = FileWindow_new();
    FileWindowIVARS *const window_ivars = FileWindow_IVARS(window);
    uint32_t i;

    remove(Str_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    for (i = 0; i < 1024; i++) {
        FSFH_Write(fh, "foo ", 4);
    }
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }

    // Reopen for reading.
    DECREF(fh);
    fh = FSFH_open(test_filename, FH_READ_ONLY);
    if (!fh) { RETHROW(INCREF(Err_get_error())); }

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Window(fh, window, -1, 4),
               "Window() with a negative offset returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Window() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(runner, FSFH_Window(fh, window, 4000, 1000),
               "Window() past EOF returns false");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Window() past EOF sets error");

    TEST_TRUE(runner, FSFH_Window(fh, window, 1021, 2),
              "Window() returns true");
    TEST_TRUE(runner,
              strncmp(window_ivars->buf - window_ivars->offset + 1021, "oo", 2) == 0,
              "Window()");

    TEST_TRUE(runner, FSFH_Release_Window(fh, window),
              "Release_Window() returns true");
    TEST_TRUE(runner, window_ivars->buf == NULL, "Release_Window() resets buf");
    TEST_TRUE(runner, window_ivars->offset == 0, "Release_Window() resets offset");
    TEST_TRUE(runner, window_ivars->len == 0, "Release_Window() resets len");

    DECREF(window);
    DECREF(fh);
    remove(Str_Get_Ptr8(test_filename));
}

void
TestFSFH_Run_IMP(TestFSFileHandle *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 46);
    test_open(runner);
    test_Read_Write(runner);
    test_Close(runner);
    test_Window(runner);
}


