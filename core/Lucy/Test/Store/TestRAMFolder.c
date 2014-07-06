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

#define C_LUCY_RAMFOLDER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestRAMFolder.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/RAMDirHandle.h"
#include "Lucy/Store/RAMFileHandle.h"

static String *foo           = NULL;
static String *bar           = NULL;
static String *baz           = NULL;
static String *boffo         = NULL;
static String *banana        = NULL;
static String *foo_bar       = NULL;
static String *foo_bar_baz   = NULL;
static String *foo_bar_boffo = NULL;
static String *foo_boffo     = NULL;
static String *foo_foo       = NULL;
static String *nope          = NULL;
static String *nope_nyet     = NULL;

TestRAMFolder*
TestRAMFolder_new() {
    return (TestRAMFolder*)Class_Make_Obj(TESTRAMFOLDER);
}

static void
S_init_strings(void) {
    foo           = Str_newf("foo");
    bar           = Str_newf("bar");
    baz           = Str_newf("baz");
    boffo         = Str_newf("boffo");
    banana        = Str_newf("banana");
    foo_bar       = Str_newf("foo/bar");
    foo_bar_baz   = Str_newf("foo/bar/baz");
    foo_bar_boffo = Str_newf("foo/bar/boffo");
    foo_boffo     = Str_newf("foo/boffo");
    foo_foo       = Str_newf("foo/foo");
    nope          = Str_newf("nope");
    nope_nyet     = Str_newf("nope/nyet");
}

static void
S_destroy_strings(void) {
    DECREF(foo);
    DECREF(bar);
    DECREF(baz);
    DECREF(boffo);
    DECREF(banana);
    DECREF(foo_bar);
    DECREF(foo_bar_baz);
    DECREF(foo_bar_boffo);
    DECREF(foo_boffo);
    DECREF(foo_foo);
    DECREF(nope);
    DECREF(nope_nyet);
}

static void
test_Initialize_and_Check(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    RAMFolder_Initialize(folder);
    PASS(runner, "Initialized concludes without incident");
    TEST_TRUE(runner, RAMFolder_Check(folder), "Check succeeds");
    DECREF(folder);
}

static void
test_Local_Exists(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh = RAMFolder_Open_FileHandle(folder, boffo,
                                               FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    RAMFolder_Local_MkDir(folder, foo);

    TEST_TRUE(runner, RAMFolder_Local_Exists(folder, boffo),
              "Local_Exists() returns true for file");
    TEST_TRUE(runner, RAMFolder_Local_Exists(folder, foo),
              "Local_Exists() returns true for dir");
    TEST_FALSE(runner, RAMFolder_Local_Exists(folder, bar),
               "Local_Exists() returns false for non-existent entry");

    DECREF(folder);
}

static void
test_Local_Is_Directory(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh = RAMFolder_Open_FileHandle(folder, boffo,
                                               FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    RAMFolder_Local_MkDir(folder, foo);

    TEST_FALSE(runner, RAMFolder_Local_Is_Directory(folder, boffo),
               "Local_Is_Directory() returns false for file");
    TEST_TRUE(runner, RAMFolder_Local_Is_Directory(folder, foo),
              "Local_Is_Directory() returns true for dir");
    TEST_FALSE(runner, RAMFolder_Local_Is_Directory(folder, bar),
               "Local_Is_Directory() returns false for non-existent entry");

    DECREF(folder);
}

static void
test_Local_Find_Folder(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    RAMFolder *local;
    FileHandle *fh;

    RAMFolder_MkDir(folder, foo);
    RAMFolder_MkDir(folder, foo_bar);
    fh = RAMFolder_Open_FileHandle(folder, boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = RAMFolder_Open_FileHandle(folder, foo_boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, nope);
    TEST_TRUE(runner, local == NULL, "Non-existent entry yields NULL");

    StackString *empty = SSTR_BLANK();
    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, (String*)empty);
    TEST_TRUE(runner, local == NULL, "Empty string yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, foo_bar);
    TEST_TRUE(runner, local == NULL, "nested folder yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, foo_boffo);
    TEST_TRUE(runner, local == NULL, "nested file yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, boffo);
    TEST_TRUE(runner, local == NULL, "local file yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, bar);
    TEST_TRUE(runner, local == NULL, "name of nested folder yields NULL");

    local = (RAMFolder*)RAMFolder_Local_Find_Folder(folder, foo);
    TEST_TRUE(runner,
              local
              && RAMFolder_Is_A(local, RAMFOLDER)
              && Str_Equals_Utf8(RAMFolder_Get_Path(local), "foo", 3),
              "Find local directory");

    DECREF(folder);
}

static void
test_Local_MkDir(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    bool result;

    result = RAMFolder_Local_MkDir(folder, foo);
    TEST_TRUE(runner, result, "Local_MkDir succeeds and returns true");

    Err_set_error(NULL);
    result = RAMFolder_Local_MkDir(folder, foo);
    TEST_FALSE(runner, result,
               "Local_MkDir returns false when a dir already exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_MkDir sets Err_error when a dir already exists");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo),
              "Existing dir untouched after failed Local_MkDir");

    FileHandle *fh = RAMFolder_Open_FileHandle(folder, boffo,
                                               FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    Err_set_error(NULL);
    result = RAMFolder_Local_MkDir(folder, foo);
    TEST_FALSE(runner, result,
               "Local_MkDir returns false when a file already exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_MkDir sets Err_error when a file already exists");
    TEST_TRUE(runner, RAMFolder_Exists(folder, boffo) &&
              !RAMFolder_Local_Is_Directory(folder, boffo),
              "Existing file untouched after failed Local_MkDir");

    DECREF(folder);
}

static void
test_Local_Open_Dir(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    DirHandle *dh = RAMFolder_Local_Open_Dir(folder);
    TEST_TRUE(runner, dh && DH_Is_A(dh, RAMDIRHANDLE),
              "Local_Open_Dir returns a RAMDirHandle");
    DECREF(dh);
    DECREF(folder);
}

static void
test_Local_Open_FileHandle(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;

    fh = RAMFolder_Local_Open_FileHandle(folder, boffo,
                                         FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, RAMFILEHANDLE),
              "opened FileHandle");
    DECREF(fh);

    fh = RAMFolder_Local_Open_FileHandle(folder, boffo,
                                         FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, RAMFILEHANDLE),
              "opened FileHandle for append");
    DECREF(fh);

    Err_set_error(NULL);
    fh = RAMFolder_Local_Open_FileHandle(folder, boffo,
                                         FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, fh == NULL, "FH_EXLUSIVE flag prevents open");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "failure due to FH_EXLUSIVE flag sets Err_error");

    fh = RAMFolder_Local_Open_FileHandle(folder, boffo, FH_READ_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, RAMFILEHANDLE),
              "opened FileHandle for reading");
    DECREF(fh);

    Err_set_error(NULL);
    fh = RAMFolder_Local_Open_FileHandle(folder, nope, FH_READ_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Can't open non-existent file for reading");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Opening non-existent file for reading sets Err_error");

    DECREF(folder);
}

static void
test_Local_Delete(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;

    fh = RAMFolder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    TEST_TRUE(runner, RAMFolder_Local_Delete(folder, boffo),
              "Local_Delete on file succeeds");

    RAMFolder_Local_MkDir(folder, foo);
    fh = RAMFolder_Open_FileHandle(folder, foo_boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    Err_set_error(NULL);
    TEST_FALSE(runner, RAMFolder_Local_Delete(folder, foo),
               "Local_Delete on non-empty dir fails");

    RAMFolder_Delete(folder, foo_boffo);
    TEST_TRUE(runner, RAMFolder_Local_Delete(folder, foo),
              "Local_Delete on empty dir succeeds");

    DECREF(folder);
}

static void
test_Rename(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;
    bool result;

    RAMFolder_MkDir(folder, foo);
    RAMFolder_MkDir(folder, foo_bar);
    fh = RAMFolder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Move files.

    result = RAMFolder_Rename(folder, boffo, banana);
    TEST_TRUE(runner, result, "Rename succeeds and returns true");
    TEST_TRUE(runner, RAMFolder_Exists(folder, banana),
              "File exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, boffo),
               "File no longer exists at old path");

    result = RAMFolder_Rename(folder, banana, foo_bar_boffo);
    TEST_TRUE(runner, result, "Rename to file in nested dir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar_boffo),
              "File exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, banana),
               "File no longer exists at old path");

    result = RAMFolder_Rename(folder, foo_bar_boffo, boffo);
    TEST_TRUE(runner, result, "Rename from file in nested dir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, boffo),
              "File exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, foo_bar_boffo),
               "File no longer exists at old path");

    fh = RAMFolder_Open_FileHandle(folder, foo_boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    result = RAMFolder_Rename(folder, boffo, foo_boffo);
    TEST_TRUE(runner, result, "Clobber");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, boffo),
               "File no longer exists at old path");

    // Move Dirs.

    RAMFolder_MkDir(folder, baz);
    result = RAMFolder_Rename(folder, baz, boffo);
    TEST_TRUE(runner, result, "Rename dir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, boffo),
              "Folder exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, baz),
               "Folder no longer exists at old path");

    result = RAMFolder_Rename(folder, boffo, foo_foo);
    TEST_TRUE(runner, result, "Rename dir into nested subdir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_foo),
              "Folder exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, boffo),
               "Folder no longer exists at old path");

    result = RAMFolder_Rename(folder, foo_foo, foo_bar_baz);
    TEST_TRUE(runner, result, "Rename dir from nested subdir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar_baz),
              "Folder exists at new path");
    TEST_FALSE(runner, RAMFolder_Exists(folder, foo_foo),
               "Folder no longer exists at old path");

    // Test failed clobbers.

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, foo_boffo, foo_bar);
    TEST_FALSE(runner, result, "Rename file clobbering dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists at old path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar),
              "Dir still exists after failed clobber");

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, foo_bar, foo_boffo);
    TEST_FALSE(runner, result, "Rename dir clobbering file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar),
              "Dir still exists at old path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists after failed clobber");

    // Test that "renaming" succeeds where to and from are the same.

    result = RAMFolder_Rename(folder, foo_boffo, foo_boffo);
    TEST_TRUE(runner, result, "Renaming file to itself succeeds");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists");

    result = RAMFolder_Rename(folder, foo_bar, foo_bar);
    TEST_TRUE(runner, result, "Renaming dir to itself succeeds");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar),
              "Dir still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, foo_boffo, nope_nyet);
    TEST_FALSE(runner, result, "Rename into non-existent subdir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Renaming into non-existent subdir sets Err_error");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, nope_nyet, boffo);
    TEST_FALSE(runner, result, "Rename non-existent file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Renaming non-existent source file sets Err_error");

    DECREF(folder);
}

static void
test_Hard_Link(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    FileHandle *fh;
    bool result;

    RAMFolder_MkDir(folder, foo);
    RAMFolder_MkDir(folder, foo_bar);
    fh = RAMFolder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    // Link files.

    result = RAMFolder_Hard_Link(folder, boffo, banana);
    TEST_TRUE(runner, result, "Hard_Link succeeds and returns true");
    TEST_TRUE(runner, RAMFolder_Exists(folder, banana),
              "File exists at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, boffo),
              "File still exists at old path");
    RAMFolder_Delete(folder, boffo);

    result = RAMFolder_Hard_Link(folder, banana, foo_bar_boffo);
    TEST_TRUE(runner, result, "Hard_Link to target within nested dir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar_boffo),
              "File exists at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, banana),
              "File still exists at old path");
    RAMFolder_Delete(folder, banana);

    result = RAMFolder_Hard_Link(folder, foo_bar_boffo, foo_boffo);
    TEST_TRUE(runner, result, "Hard_Link from file in nested dir");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File exists at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_bar_boffo),
              "File still exists at old path");
    RAMFolder_Delete(folder, foo_bar_boffo);

    // Invalid clobbers.

    fh = RAMFolder_Open_FileHandle(folder, boffo, FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    result = RAMFolder_Hard_Link(folder, foo_boffo, boffo);
    TEST_FALSE(runner, result, "Clobber of file fails");
    TEST_TRUE(runner, RAMFolder_Exists(folder, boffo),
              "File still exists at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists at old path");
    RAMFolder_Delete(folder, boffo);

    RAMFolder_MkDir(folder, baz);
    result = RAMFolder_Hard_Link(folder, foo_boffo, baz);
    TEST_FALSE(runner, result, "Clobber of dir fails");
    TEST_TRUE(runner, RAMFolder_Exists(folder, baz),
              "Dir still exists at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists at old path");
    RAMFolder_Delete(folder, baz);

    // Invalid Hard_Link of dir.

    RAMFolder_MkDir(folder, baz);
    result = RAMFolder_Hard_Link(folder, baz, banana);
    TEST_FALSE(runner, result, "Hard_Link dir fails");
    TEST_FALSE(runner, RAMFolder_Exists(folder, banana),
               "Nothing at new path");
    TEST_TRUE(runner, RAMFolder_Exists(folder, baz),
              "Folder still exists at old path");
    RAMFolder_Delete(folder, baz);

    // Test that linking to yourself fails.

    result = RAMFolder_Hard_Link(folder, foo_boffo, foo_boffo);
    TEST_FALSE(runner, result, "Hard_Link file to itself fails");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "File still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, foo_boffo, nope_nyet);
    TEST_FALSE(runner, result, "Hard_Link into non-existent subdir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Hard_Link into non-existent subdir sets Err_error");
    TEST_TRUE(runner, RAMFolder_Exists(folder, foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = RAMFolder_Rename(folder, nope_nyet, boffo);
    TEST_FALSE(runner, result, "Hard_Link non-existent source file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Hard_Link non-existent source file sets Err_error");

    DECREF(folder);
}

static void
test_Close(TestBatchRunner *runner) {
    RAMFolder *folder = RAMFolder_new(NULL);
    RAMFolder_Close(folder);
    PASS(runner, "Close() concludes without incident");
    RAMFolder_Close(folder);
    RAMFolder_Close(folder);
    PASS(runner, "Calling Close() multiple times is safe");
    DECREF(folder);
}

void
TestRAMFolder_Run_IMP(TestRAMFolder *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 98);
    S_init_strings();
    test_Initialize_and_Check(runner);
    test_Local_Exists(runner);
    test_Local_Is_Directory(runner);
    test_Local_Find_Folder(runner);
    test_Local_MkDir(runner);
    test_Local_Open_Dir(runner);
    test_Local_Open_FileHandle(runner);
    test_Local_Delete(runner);
    test_Rename(runner);
    test_Hard_Link(runner);
    test_Close(runner);
    S_destroy_strings();
}


