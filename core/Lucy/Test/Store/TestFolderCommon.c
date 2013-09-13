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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFolderCommon.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

#define set_up_t    LUCY_TestFolderCommon_Set_Up_t
#define tear_down_t LUCY_TestFolderCommon_Tear_Down_t

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
test_Local_Exists(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    Folder_Local_MkDir(folder, foo);
    outstream = Folder_Open_Out(folder, foo_boffo);
    DECREF(outstream);

    TEST_TRUE(runner, Folder_Local_Exists(folder, boffo),
              "Local_Exists() returns true for file");
    TEST_TRUE(runner, Folder_Local_Exists(folder, foo),
              "Local_Exists() returns true for dir");
    TEST_FALSE(runner, Folder_Local_Exists(folder, foo_boffo),
               "Local_Exists() returns false for nested entry");
    TEST_FALSE(runner, Folder_Local_Exists(folder, bar),
               "Local_Exists() returns false for non-existent entry");

    Folder_Delete(folder, foo_boffo);
    Folder_Delete(folder, foo);
    Folder_Delete(folder, boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Is_Directory(TestBatchRunner *runner, set_up_t set_up,
                        tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    Folder_Local_MkDir(folder, foo);

    TEST_FALSE(runner, Folder_Local_Is_Directory(folder, boffo),
               "Local_Is_Directory() returns false for file");
    TEST_TRUE(runner, Folder_Local_Is_Directory(folder, foo),
              "Local_Is_Directory() returns true for dir");
    TEST_FALSE(runner, Folder_Local_Is_Directory(folder, bar),
               "Local_Is_Directory() returns false for non-existent entry");

    Folder_Delete(folder, boffo);
    Folder_Delete(folder, foo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Find_Folder(TestBatchRunner *runner, set_up_t set_up,
                       tear_down_t tear_down) {
    Folder    *folder = set_up();
    Folder    *local;
    OutStream *outstream;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    outstream = Folder_Open_Out(folder, foo_boffo);
    DECREF(outstream);

    local = Folder_Local_Find_Folder(folder, nope);
    TEST_TRUE(runner, local == NULL, "Non-existent entry yields NULL");

    StackString *empty = SSTR_BLANK();
    local = Folder_Local_Find_Folder(folder, (String*)empty);
    TEST_TRUE(runner, local == NULL, "Empty string yields NULL");

    local = Folder_Local_Find_Folder(folder, foo_bar);
    TEST_TRUE(runner, local == NULL, "nested folder yields NULL");

    local = Folder_Local_Find_Folder(folder, foo_boffo);
    TEST_TRUE(runner, local == NULL, "nested file yields NULL");

    local = Folder_Local_Find_Folder(folder, boffo);
    TEST_TRUE(runner, local == NULL, "local file yields NULL");

    local = Folder_Local_Find_Folder(folder, bar);
    TEST_TRUE(runner, local == NULL, "name of nested folder yields NULL");

    local = Folder_Local_Find_Folder(folder, foo);
    TEST_TRUE(runner,
              local
              && Folder_Is_A(local, FOLDER)
              && Str_Ends_With(Folder_Get_Path(local), foo),
              "Find local directory");

    Folder_Delete(folder, foo_bar);
    Folder_Delete(folder, foo_boffo);
    Folder_Delete(folder, foo);
    Folder_Delete(folder, boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_MkDir(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    bool result;

    result = Folder_Local_MkDir(folder, foo);
    TEST_TRUE(runner, result, "Local_MkDir succeeds and returns true");

    Err_set_error(NULL);
    result = Folder_Local_MkDir(folder, foo);
    TEST_FALSE(runner, result,
               "Local_MkDir returns false when a dir already exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_MkDir sets Err_error when a dir already exists");
    TEST_TRUE(runner, Folder_Exists(folder, foo),
              "Existing dir untouched after failed Local_MkDir");

    OutStream *outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    Err_set_error(NULL);
    result = Folder_Local_MkDir(folder, foo);
    TEST_FALSE(runner, result,
               "Local_MkDir returns false when a file already exists");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Local_MkDir sets Err_error when a file already exists");
    TEST_TRUE(runner, Folder_Exists(folder, boffo) &&
              !Folder_Local_Is_Directory(folder, boffo),
              "Existing file untouched after failed Local_MkDir");

    Folder_Delete(folder, foo);
    Folder_Delete(folder, boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Open_Dir(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    DirHandle *dh = Folder_Local_Open_Dir(folder);
    TEST_TRUE(runner, dh && DH_Is_A(dh, DIRHANDLE),
              "Local_Open_Dir returns an DirHandle");
    DECREF(dh);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Open_FileHandle(TestBatchRunner *runner, set_up_t set_up,
                           tear_down_t tear_down) {
    Folder *folder = set_up();
    FileHandle *fh;

    fh = Folder_Local_Open_FileHandle(folder, boffo,
                                      FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle");
    DECREF(fh);

    fh = Folder_Local_Open_FileHandle(folder, boffo,
                                      FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle for append");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, boffo,
                                      FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(runner, fh == NULL, "FH_EXLUSIVE flag prevents clobber");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "failure due to FH_EXLUSIVE flag sets Err_error");

    fh = Folder_Local_Open_FileHandle(folder, boffo, FH_READ_ONLY);
    TEST_TRUE(runner, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle for reading");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, nope, FH_READ_ONLY);
    TEST_TRUE(runner, fh == NULL,
              "Can't open non-existent file for reading");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Opening non-existent file for reading sets Err_error");

    Folder_Delete(folder, boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Delete(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;

    outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    TEST_TRUE(runner, Folder_Local_Delete(folder, boffo),
              "Local_Delete on file succeeds");
    TEST_FALSE(runner, Folder_Exists(folder, boffo),
               "File is really gone");

    Folder_Local_MkDir(folder, foo);
    outstream = Folder_Open_Out(folder, foo_boffo);
    DECREF(outstream);

    Err_set_error(NULL);
    TEST_FALSE(runner, Folder_Local_Delete(folder, foo),
               "Local_Delete on non-empty dir fails");

    Folder_Delete(folder, foo_boffo);
    TEST_TRUE(runner, Folder_Local_Delete(folder, foo),
              "Local_Delete on empty dir succeeds");
    TEST_FALSE(runner, Folder_Exists(folder, foo),
               "Dir is really gone");

    DECREF(folder);
    tear_down();
}

static void
test_Rename(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;
    bool result;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    outstream = Folder_Open_Out(folder, boffo);
    OutStream_Close(outstream);
    DECREF(outstream);

    // Move files.

    result = Folder_Rename(folder, boffo, banana);
    TEST_TRUE(runner, result, "Rename succeeds and returns true");
    TEST_TRUE(runner, Folder_Exists(folder, banana),
              "File exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, boffo),
               "File no longer exists at old path");

    result = Folder_Rename(folder, banana, foo_bar_boffo);
    TEST_TRUE(runner, result, "Rename to file in nested dir");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar_boffo),
              "File exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, banana),
               "File no longer exists at old path");

    result = Folder_Rename(folder, foo_bar_boffo, boffo);
    TEST_TRUE(runner, result, "Rename from file in nested dir");
    TEST_TRUE(runner, Folder_Exists(folder, boffo),
              "File exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, foo_bar_boffo),
               "File no longer exists at old path");

    outstream = Folder_Open_Out(folder, foo_boffo);
    OutStream_Close(outstream);
    DECREF(outstream);
    result = Folder_Rename(folder, boffo, foo_boffo);
    if (result) {
        PASS(runner, "Rename clobbers on this system");
        TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
                  "File exists at new path");
        TEST_FALSE(runner, Folder_Exists(folder, boffo),
                   "File no longer exists at old path");
    }
    else {
        PASS(runner, "Rename does not clobber on this system");
        TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
                  "File exists at new path");
        TEST_TRUE(runner, Folder_Exists(folder, boffo),
                  "File still exists at old path");
        Folder_Delete(folder, boffo);
    }

    // Move Dirs.

    Folder_MkDir(folder, baz);
    result = Folder_Rename(folder, baz, boffo);
    TEST_TRUE(runner, result, "Rename dir");
    TEST_TRUE(runner, Folder_Exists(folder, boffo),
              "Folder exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, baz),
               "Folder no longer exists at old path");

    result = Folder_Rename(folder, boffo, foo_foo);
    TEST_TRUE(runner, result, "Rename dir into nested subdir");
    TEST_TRUE(runner, Folder_Exists(folder, foo_foo),
              "Folder exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, boffo),
               "Folder no longer exists at old path");

    result = Folder_Rename(folder, foo_foo, foo_bar_baz);
    TEST_TRUE(runner, result, "Rename dir from nested subdir");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar_baz),
              "Folder exists at new path");
    TEST_FALSE(runner, Folder_Exists(folder, foo_foo),
               "Folder no longer exists at old path");

    // Test failed clobbers.

    Err_set_error(NULL);
    result = Folder_Rename(folder, foo_boffo, foo_bar);
    TEST_FALSE(runner, result, "Rename file clobbering dir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists at old path");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar),
              "Dir still exists after failed clobber");

    Err_set_error(NULL);
    result = Folder_Rename(folder, foo_bar, foo_boffo);
    TEST_FALSE(runner, result, "Rename dir clobbering file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar),
              "Dir still exists at old path");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists after failed clobber");

    // Test that "renaming" succeeds where to and from are the same.

    result = Folder_Rename(folder, foo_boffo, foo_boffo);
    TEST_TRUE(runner, result, "Renaming file to itself succeeds");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists");

    result = Folder_Rename(folder, foo_bar, foo_bar);
    TEST_TRUE(runner, result, "Renaming dir to itself succeeds");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar),
              "Dir still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = Folder_Rename(folder, foo_boffo, nope_nyet);
    TEST_FALSE(runner, result, "Rename into non-existent subdir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Renaming into non-existent subdir sets Err_error");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, nope_nyet, boffo);
    TEST_FALSE(runner, result, "Rename non-existent file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Renaming non-existent source file sets Err_error");

    Folder_Delete(folder, foo_bar_baz);
    Folder_Delete(folder, foo_bar);
    Folder_Delete(folder, foo_boffo);
    Folder_Delete(folder, foo);
    DECREF(folder);
    tear_down();
}

static void
test_Hard_Link(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;
    bool result;

    Folder_MkDir(folder, foo);
    Folder_MkDir(folder, foo_bar);
    outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);

    // Link files.

    result = Folder_Hard_Link(folder, boffo, banana);
    TEST_TRUE(runner, result, "Hard_Link succeeds and returns true");
    TEST_TRUE(runner, Folder_Exists(folder, banana),
              "File exists at new path");
    TEST_TRUE(runner, Folder_Exists(folder, boffo),
              "File still exists at old path");
    Folder_Delete(folder, boffo);

    result = Folder_Hard_Link(folder, banana, foo_bar_boffo);
    TEST_TRUE(runner, result, "Hard_Link to target within nested dir");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar_boffo),
              "File exists at new path");
    TEST_TRUE(runner, Folder_Exists(folder, banana),
              "File still exists at old path");
    Folder_Delete(folder, banana);

    result = Folder_Hard_Link(folder, foo_bar_boffo, foo_boffo);
    TEST_TRUE(runner, result, "Hard_Link from file in nested dir");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File exists at new path");
    TEST_TRUE(runner, Folder_Exists(folder, foo_bar_boffo),
              "File still exists at old path");
    Folder_Delete(folder, foo_bar_boffo);

    // Invalid clobbers.

    outstream = Folder_Open_Out(folder, boffo);
    DECREF(outstream);
    result = Folder_Hard_Link(folder, foo_boffo, boffo);
    TEST_FALSE(runner, result, "Clobber of file fails");
    TEST_TRUE(runner, Folder_Exists(folder, boffo),
              "File still exists at new path");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists at old path");
    Folder_Delete(folder, boffo);

    Folder_MkDir(folder, baz);
    result = Folder_Hard_Link(folder, foo_boffo, baz);
    TEST_FALSE(runner, result, "Clobber of dir fails");
    TEST_TRUE(runner, Folder_Exists(folder, baz),
              "Dir still exists at new path");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists at old path");
    Folder_Delete(folder, baz);

    // Invalid Hard_Link of dir.

    Folder_MkDir(folder, baz);
    result = Folder_Hard_Link(folder, baz, banana);
    TEST_FALSE(runner, result, "Hard_Link dir fails");
    TEST_FALSE(runner, Folder_Exists(folder, banana),
               "Nothing at new path");
    TEST_TRUE(runner, Folder_Exists(folder, baz),
              "Folder still exists at old path");
    Folder_Delete(folder, baz);

    // Test that linking to yourself fails.

    result = Folder_Hard_Link(folder, foo_boffo, foo_boffo);
    TEST_FALSE(runner, result, "Hard_Link file to itself fails");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "File still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = Folder_Rename(folder, foo_boffo, nope_nyet);
    TEST_FALSE(runner, result, "Hard_Link into non-existent subdir fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Hard_Link into non-existent subdir sets Err_error");
    TEST_TRUE(runner, Folder_Exists(folder, foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, nope_nyet, boffo);
    TEST_FALSE(runner, result, "Hard_Link non-existent source file fails");
    TEST_TRUE(runner, Err_get_error() != NULL,
              "Hard_Link non-existent source file sets Err_error");

    Folder_Delete(folder, foo_bar);
    Folder_Delete(folder, foo_boffo);
    Folder_Delete(folder, foo);
    DECREF(folder);
    tear_down();
}

static void
test_Close(TestBatchRunner *runner, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    Folder_Close(folder);
    PASS(runner, "Close() concludes without incident");
    Folder_Close(folder);
    Folder_Close(folder);
    PASS(runner, "Calling Close() multiple times is safe");
    DECREF(folder);
    tear_down();
}

uint32_t
TestFolderCommon_num_tests() {
    return 99;
}

void
TestFolderCommon_run_tests(TestBatchRunner *runner, set_up_t set_up,
                           tear_down_t tear_down) {
    S_init_strings();
    test_Local_Exists(runner, set_up, tear_down);
    test_Local_Is_Directory(runner, set_up, tear_down);
    test_Local_Find_Folder(runner, set_up, tear_down);
    test_Local_MkDir(runner, set_up, tear_down);
    test_Local_Open_Dir(runner, set_up, tear_down);
    test_Local_Open_FileHandle(runner, set_up, tear_down);
    test_Local_Delete(runner, set_up, tear_down);
    test_Rename(runner, set_up, tear_down);
    test_Hard_Link(runner, set_up, tear_down);
    test_Close(runner, set_up, tear_down);
    S_destroy_strings();
}


