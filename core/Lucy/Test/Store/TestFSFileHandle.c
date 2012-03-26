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

#define C_LUCY_CHARBUF
#define C_LUCY_FSFILEHANDLE
#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h> // close
#elif defined(CHY_HAS_IO_H)
  #include <io.h> // close
#endif

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSFileHandle.h"
#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"

static void
test_open(TestBatch *batch) {

    FSFileHandle *fh;
    CharBuf *test_filename = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);

    remove((char*)CB_Get_Ptr8(test_filename));

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "open() with FH_READ_ONLY on non-existent file returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "open() with FH_READ_ONLY on non-existent file sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_WRITE_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "open() without FH_CREATE returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "open() without FH_CREATE sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE);
    TEST_TRUE(batch, fh == NULL,
              "open() without FH_WRITE_ONLY returns NULL");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "open() without FH_WRITE_ONLY sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(batch, fh && FSFH_Is_A(fh, FSFILEHANDLE), "open() succeeds");
    TEST_TRUE(batch, Err_get_error() == NULL, "open() no errors");
    FSFH_Write(fh, "foo", 3);
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(batch, fh == NULL, "FH_EXCLUSIVE blocks open()");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "FH_EXCLUSIVE blocks open(), sets error");

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh && FSFH_Is_A(fh, FSFILEHANDLE),
              "open() for append");
    TEST_TRUE(batch, Err_get_error() == NULL,
              "open() for append -- no errors");
    FSFH_Write(fh, "bar", 3);
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(batch, fh && FSFH_Is_A(fh, FSFILEHANDLE), "open() read only");
    TEST_TRUE(batch, Err_get_error() == NULL,
              "open() read only -- no errors");
    DECREF(fh);

    remove((char*)CB_Get_Ptr8(test_filename));
}

static void
test_Read_Write(TestBatch *batch) {
    FSFileHandle *fh;
    const char *foo = "foo";
    const char *bar = "bar";
    char buffer[12];
    char *buf = buffer;
    CharBuf *test_filename = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);

    remove((char*)CB_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);

    TEST_TRUE(batch, FSFH_Length(fh) == I64_C(0), "Length initially 0");
    TEST_TRUE(batch, FSFH_Write(fh, foo, 3), "Write returns success");
    TEST_TRUE(batch, FSFH_Length(fh) == I64_C(3), "Length after Write");
    TEST_TRUE(batch, FSFH_Write(fh, bar, 3), "Write returns success");
    TEST_TRUE(batch, FSFH_Length(fh) == I64_C(6), "Length after 2 Writes");

    Err_set_error(NULL);
    TEST_FALSE(batch, FSFH_Read(fh, buf, 0, 2),
               "Reading from a write-only handle returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Reading from a write-only handle sets error");
    if (!FSFH_Close(fh)) { RETHROW(INCREF(Err_get_error())); }
    DECREF(fh);

    // Reopen for reading.
    Err_set_error(NULL);
    fh = FSFH_open(test_filename, FH_READ_ONLY);

    TEST_TRUE(batch, FSFH_Length(fh) == I64_C(6), "Length on Read");
    TEST_TRUE(batch, FSFH_Read(fh, buf, 0, 6), "Read returns success");
    TEST_TRUE(batch, strncmp(buf, "foobar", 6) == 0, "Read/Write");
    TEST_TRUE(batch, FSFH_Read(fh, buf, 2, 3), "Read returns success");
    TEST_TRUE(batch, strncmp(buf, "oba", 3) == 0, "Read with offset");

    Err_set_error(NULL);
    TEST_FALSE(batch, FSFH_Read(fh, buf, -1, 4),
               "Read() with a negative offset returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Read() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(batch, FSFH_Read(fh, buf, 6, 1),
               "Read() past EOF returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Read() past EOF sets error");

    Err_set_error(NULL);
    TEST_FALSE(batch, FSFH_Write(fh, foo, 3),
               "Writing to a read-only handle returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Writing to a read-only handle sets error");

    DECREF(fh);
    remove((char*)CB_Get_Ptr8(test_filename));
}

static void
test_Close(TestBatch *batch) {
    CharBuf *test_filename = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);
    FSFileHandle *fh;

    remove((char*)CB_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(batch, FSFH_Close(fh), "Close returns true for write-only");
    DECREF(fh);

    // Simulate an OS error when closing the file descriptor.  This
    // approximates what would happen if, say, we run out of disk space.
    remove((char*)CB_Get_Ptr8(test_filename));
    fh = FSFH_open(test_filename,
                   FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
#ifdef _MSC_VER
    SKIP(batch, "LUCY-155");
    SKIP(batch, "LUCY-155");
#else
    int saved_fd = fh->fd;
    fh->fd = -1;
    Err_set_error(NULL);
    bool_t result = FSFH_Close(fh);
    TEST_FALSE(batch, result, "Failed Close() returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed Close() sets Err_error");
    fh->fd = saved_fd;
#endif /* _MSC_VER */
    DECREF(fh);

    fh = FSFH_open(test_filename, FH_READ_ONLY);
    TEST_TRUE(batch, FSFH_Close(fh), "Close returns true for read-only");

    DECREF(fh);
    remove((char*)CB_Get_Ptr8(test_filename));
}

static void
test_Window(TestBatch *batch) {
    CharBuf *test_filename = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);
    FSFileHandle *fh;
    FileWindow *window = FileWindow_new();
    uint32_t i;

    remove((char*)CB_Get_Ptr8(test_filename));
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
    TEST_FALSE(batch, FSFH_Window(fh, window, -1, 4),
               "Window() with a negative offset returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Window() with a negative offset sets error");

    Err_set_error(NULL);
    TEST_FALSE(batch, FSFH_Window(fh, window, 4000, 1000),
               "Window() past EOF returns false");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Window() past EOF sets error");

    TEST_TRUE(batch, FSFH_Window(fh, window, 1021, 2),
              "Window() returns true");
    TEST_TRUE(batch,
              strncmp(window->buf - window->offset + 1021, "oo", 2) == 0,
              "Window()");

    TEST_TRUE(batch, FSFH_Release_Window(fh, window),
              "Release_Window() returns true");
    TEST_TRUE(batch, window->buf == NULL, "Release_Window() resets buf");
    TEST_TRUE(batch, window->offset == 0, "Release_Window() resets offset");
    TEST_TRUE(batch, window->len == 0, "Release_Window() resets len");

    DECREF(window);
    DECREF(fh);
    remove((char*)CB_Get_Ptr8(test_filename));
}

void
TestFSFH_run_tests() {
    TestBatch *batch = TestBatch_new(46);

    TestBatch_Plan(batch);
    test_open(batch);
    test_Read_Write(batch);
    test_Close(batch);
    test_Window(batch);

    DECREF(batch);
}


