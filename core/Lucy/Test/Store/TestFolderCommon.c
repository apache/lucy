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

#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFolderCommon.h"
#include "Lucy/Store/Folder.h"
#include "Lucy/Store/DirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"

#define set_up_t    lucy_TestFolderCommon_set_up_t
#define tear_down_t lucy_TestFolderCommon_tear_down_t

static CharBuf foo           = ZCB_LITERAL("foo");
static CharBuf bar           = ZCB_LITERAL("bar");
static CharBuf baz           = ZCB_LITERAL("baz");
static CharBuf boffo         = ZCB_LITERAL("boffo");
static CharBuf banana        = ZCB_LITERAL("banana");
static CharBuf foo_bar       = ZCB_LITERAL("foo/bar");
static CharBuf foo_bar_baz   = ZCB_LITERAL("foo/bar/baz");
static CharBuf foo_bar_boffo = ZCB_LITERAL("foo/bar/boffo");
static CharBuf foo_boffo     = ZCB_LITERAL("foo/boffo");
static CharBuf foo_foo       = ZCB_LITERAL("foo/foo");
static CharBuf nope          = ZCB_LITERAL("nope");
static CharBuf nope_nyet     = ZCB_LITERAL("nope/nyet");

static void
test_Local_Exists(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);
    Folder_Local_MkDir(folder, &foo);
    outstream = Folder_Open_Out(folder, &foo_boffo);
    DECREF(outstream);

    TEST_TRUE(batch, Folder_Local_Exists(folder, &boffo),
              "Local_Exists() returns true for file");
    TEST_TRUE(batch, Folder_Local_Exists(folder, &foo),
              "Local_Exists() returns true for dir");
    TEST_FALSE(batch, Folder_Local_Exists(folder, &foo_boffo),
               "Local_Exists() returns false for nested entry");
    TEST_FALSE(batch, Folder_Local_Exists(folder, &bar),
               "Local_Exists() returns false for non-existent entry");

    Folder_Delete(folder, &foo_boffo);
    Folder_Delete(folder, &foo);
    Folder_Delete(folder, &boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Is_Directory(TestBatch *batch, set_up_t set_up,
                        tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);
    Folder_Local_MkDir(folder, &foo);

    TEST_FALSE(batch, Folder_Local_Is_Directory(folder, &boffo),
               "Local_Is_Directory() returns false for file");
    TEST_TRUE(batch, Folder_Local_Is_Directory(folder, &foo),
              "Local_Is_Directory() returns true for dir");
    TEST_FALSE(batch, Folder_Local_Is_Directory(folder, &bar),
               "Local_Is_Directory() returns false for non-existent entry");

    Folder_Delete(folder, &boffo);
    Folder_Delete(folder, &foo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Find_Folder(TestBatch *batch, set_up_t set_up,
                       tear_down_t tear_down) {
    Folder    *folder = set_up();
    Folder    *local;
    OutStream *outstream;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);
    outstream = Folder_Open_Out(folder, &foo_boffo);
    DECREF(outstream);

    local = Folder_Local_Find_Folder(folder, &nope);
    TEST_TRUE(batch, local == NULL, "Non-existent entry yields NULL");

    local = Folder_Local_Find_Folder(folder, (CharBuf*)&EMPTY);
    TEST_TRUE(batch, local == NULL, "Empty string yields NULL");

    local = Folder_Local_Find_Folder(folder, &foo_bar);
    TEST_TRUE(batch, local == NULL, "nested folder yields NULL");

    local = Folder_Local_Find_Folder(folder, &foo_boffo);
    TEST_TRUE(batch, local == NULL, "nested file yields NULL");

    local = Folder_Local_Find_Folder(folder, &boffo);
    TEST_TRUE(batch, local == NULL, "local file yields NULL");

    local = Folder_Local_Find_Folder(folder, &bar);
    TEST_TRUE(batch, local == NULL, "name of nested folder yields NULL");

    local = Folder_Local_Find_Folder(folder, &foo);
    TEST_TRUE(batch,
              local
              && Folder_Is_A(local, FOLDER)
              && CB_Ends_With(Folder_Get_Path(local), &foo),
              "Find local directory");

    Folder_Delete(folder, &foo_bar);
    Folder_Delete(folder, &foo_boffo);
    Folder_Delete(folder, &foo);
    Folder_Delete(folder, &boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_MkDir(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    bool_t result;

    result = Folder_Local_MkDir(folder, &foo);
    TEST_TRUE(batch, result, "Local_MkDir succeeds and returns true");

    Err_set_error(NULL);
    result = Folder_Local_MkDir(folder, &foo);
    TEST_FALSE(batch, result,
               "Local_MkDir returns false when a dir already exists");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Local_MkDir sets Err_error when a dir already exists");
    TEST_TRUE(batch, Folder_Exists(folder, &foo),
              "Existing dir untouched after failed Local_MkDir");

    {
        OutStream *outstream = Folder_Open_Out(folder, &boffo);
        DECREF(outstream);
        Err_set_error(NULL);
        result = Folder_Local_MkDir(folder, &foo);
        TEST_FALSE(batch, result,
                   "Local_MkDir returns false when a file already exists");
        TEST_TRUE(batch, Err_get_error() != NULL,
                  "Local_MkDir sets Err_error when a file already exists");
        TEST_TRUE(batch, Folder_Exists(folder, &boffo) &&
                  !Folder_Local_Is_Directory(folder, &boffo),
                  "Existing file untouched after failed Local_MkDir");
    }

    Folder_Delete(folder, &foo);
    Folder_Delete(folder, &boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Open_Dir(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    DirHandle *dh = Folder_Local_Open_Dir(folder);
    TEST_TRUE(batch, dh && DH_Is_A(dh, DIRHANDLE),
              "Local_Open_Dir returns an DirHandle");
    DECREF(dh);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Open_FileHandle(TestBatch *batch, set_up_t set_up,
                           tear_down_t tear_down) {
    Folder *folder = set_up();
    FileHandle *fh;

    fh = Folder_Local_Open_FileHandle(folder, &boffo,
                                      FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(batch, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle");
    DECREF(fh);

    fh = Folder_Local_Open_FileHandle(folder, &boffo,
                                      FH_CREATE | FH_WRITE_ONLY);
    TEST_TRUE(batch, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle for append");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, &boffo,
                                      FH_CREATE | FH_WRITE_ONLY | FH_EXCLUSIVE);
    TEST_TRUE(batch, fh == NULL, "FH_EXLUSIVE flag prevents clobber");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "failure due to FH_EXLUSIVE flag sets Err_error");

    fh = Folder_Local_Open_FileHandle(folder, &boffo, FH_READ_ONLY);
    TEST_TRUE(batch, fh && FH_Is_A(fh, FILEHANDLE),
              "opened FileHandle for reading");
    DECREF(fh);

    Err_set_error(NULL);
    fh = Folder_Local_Open_FileHandle(folder, &nope, FH_READ_ONLY);
    TEST_TRUE(batch, fh == NULL,
              "Can't open non-existent file for reading");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Opening non-existent file for reading sets Err_error");

    Folder_Delete(folder, &boffo);
    DECREF(folder);
    tear_down();
}

static void
test_Local_Delete(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;

    outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);
    TEST_TRUE(batch, Folder_Local_Delete(folder, &boffo),
              "Local_Delete on file succeeds");
    TEST_FALSE(batch, Folder_Exists(folder, &boffo),
               "File is really gone");

    Folder_Local_MkDir(folder, &foo);
    outstream = Folder_Open_Out(folder, &foo_boffo);
    DECREF(outstream);

    Err_set_error(NULL);
    TEST_FALSE(batch, Folder_Local_Delete(folder, &foo),
               "Local_Delete on non-empty dir fails");

    Folder_Delete(folder, &foo_boffo);
    TEST_TRUE(batch, Folder_Local_Delete(folder, &foo),
              "Local_Delete on empty dir succeeds");
    TEST_FALSE(batch, Folder_Exists(folder, &foo),
               "Dir is really gone");

    DECREF(folder);
    tear_down();
}

static void
test_Rename(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;
    bool_t result;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    outstream = Folder_Open_Out(folder, &boffo);
    OutStream_Close(outstream);
    DECREF(outstream);

    // Move files.

    result = Folder_Rename(folder, &boffo, &banana);
    TEST_TRUE(batch, result, "Rename succeeds and returns true");
    TEST_TRUE(batch, Folder_Exists(folder, &banana),
              "File exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &boffo),
               "File no longer exists at old path");

    result = Folder_Rename(folder, &banana, &foo_bar_boffo);
    TEST_TRUE(batch, result, "Rename to file in nested dir");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo),
              "File exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &banana),
               "File no longer exists at old path");

    result = Folder_Rename(folder, &foo_bar_boffo, &boffo);
    TEST_TRUE(batch, result, "Rename from file in nested dir");
    TEST_TRUE(batch, Folder_Exists(folder, &boffo),
              "File exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_bar_boffo),
               "File no longer exists at old path");

    outstream = Folder_Open_Out(folder, &foo_boffo);
    OutStream_Close(outstream);
    DECREF(outstream);
    result = Folder_Rename(folder, &boffo, &foo_boffo);
    if (result) {
        PASS(batch, "Rename clobbers on this system");
        TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
                  "File exists at new path");
        TEST_FALSE(batch, Folder_Exists(folder, &boffo),
                   "File no longer exists at old path");
    }
    else {
        PASS(batch, "Rename does not clobber on this system");
        TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
                  "File exists at new path");
        TEST_TRUE(batch, Folder_Exists(folder, &boffo),
                  "File still exists at old path");
        Folder_Delete(folder, &boffo);
    }

    // Move Dirs.

    Folder_MkDir(folder, &baz);
    result = Folder_Rename(folder, &baz, &boffo);
    TEST_TRUE(batch, result, "Rename dir");
    TEST_TRUE(batch, Folder_Exists(folder, &boffo),
              "Folder exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &baz),
               "Folder no longer exists at old path");

    result = Folder_Rename(folder, &boffo, &foo_foo);
    TEST_TRUE(batch, result, "Rename dir into nested subdir");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_foo),
              "Folder exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &boffo),
               "Folder no longer exists at old path");

    result = Folder_Rename(folder, &foo_foo, &foo_bar_baz);
    TEST_TRUE(batch, result, "Rename dir from nested subdir");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar_baz),
              "Folder exists at new path");
    TEST_FALSE(batch, Folder_Exists(folder, &foo_foo),
               "Folder no longer exists at old path");

    // Test failed clobbers.

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &foo_bar);
    TEST_FALSE(batch, result, "Rename file clobbering dir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists at old path");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar),
              "Dir still exists after failed clobber");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_bar, &foo_boffo);
    TEST_FALSE(batch, result, "Rename dir clobbering file fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Failed rename sets Err_error");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar),
              "Dir still exists at old path");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists after failed clobber");

    // Test that "renaming" succeeds where to and from are the same.

    result = Folder_Rename(folder, &foo_boffo, &foo_boffo);
    TEST_TRUE(batch, result, "Renaming file to itself succeeds");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists");

    result = Folder_Rename(folder, &foo_bar, &foo_bar);
    TEST_TRUE(batch, result, "Renaming dir to itself succeeds");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar),
              "Dir still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &nope_nyet);
    TEST_FALSE(batch, result, "Rename into non-existent subdir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Renaming into non-existent subdir sets Err_error");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &nope_nyet, &boffo);
    TEST_FALSE(batch, result, "Rename non-existent file fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Renaming non-existent source file sets Err_error");

    Folder_Delete(folder, &foo_bar_baz);
    Folder_Delete(folder, &foo_bar);
    Folder_Delete(folder, &foo_boffo);
    Folder_Delete(folder, &foo);
    DECREF(folder);
    tear_down();
}

static void
test_Hard_Link(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    OutStream *outstream;
    bool_t result;

    Folder_MkDir(folder, &foo);
    Folder_MkDir(folder, &foo_bar);
    outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);

    // Link files.

    result = Folder_Hard_Link(folder, &boffo, &banana);
    TEST_TRUE(batch, result, "Hard_Link succeeds and returns true");
    TEST_TRUE(batch, Folder_Exists(folder, &banana),
              "File exists at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &boffo),
              "File still exists at old path");
    Folder_Delete(folder, &boffo);

    result = Folder_Hard_Link(folder, &banana, &foo_bar_boffo);
    TEST_TRUE(batch, result, "Hard_Link to target within nested dir");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo),
              "File exists at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &banana),
              "File still exists at old path");
    Folder_Delete(folder, &banana);

    result = Folder_Hard_Link(folder, &foo_bar_boffo, &foo_boffo);
    TEST_TRUE(batch, result, "Hard_Link from file in nested dir");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File exists at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_bar_boffo),
              "File still exists at old path");
    Folder_Delete(folder, &foo_bar_boffo);

    // Invalid clobbers.

    outstream = Folder_Open_Out(folder, &boffo);
    DECREF(outstream);
    result = Folder_Hard_Link(folder, &foo_boffo, &boffo);
    TEST_FALSE(batch, result, "Clobber of file fails");
    TEST_TRUE(batch, Folder_Exists(folder, &boffo),
              "File still exists at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists at old path");
    Folder_Delete(folder, &boffo);

    Folder_MkDir(folder, &baz);
    result = Folder_Hard_Link(folder, &foo_boffo, &baz);
    TEST_FALSE(batch, result, "Clobber of dir fails");
    TEST_TRUE(batch, Folder_Exists(folder, &baz),
              "Dir still exists at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists at old path");
    Folder_Delete(folder, &baz);

    // Invalid Hard_Link of dir.

    Folder_MkDir(folder, &baz);
    result = Folder_Hard_Link(folder, &baz, &banana);
    TEST_FALSE(batch, result, "Hard_Link dir fails");
    TEST_FALSE(batch, Folder_Exists(folder, &banana),
               "Nothing at new path");
    TEST_TRUE(batch, Folder_Exists(folder, &baz),
              "Folder still exists at old path");
    Folder_Delete(folder, &baz);

    // Test that linking to yourself fails.

    result = Folder_Hard_Link(folder, &foo_boffo, &foo_boffo);
    TEST_FALSE(batch, result, "Hard_Link file to itself fails");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "File still exists");

    // Invalid filepaths.

    Err_set_error(NULL);
    result = Folder_Rename(folder, &foo_boffo, &nope_nyet);
    TEST_FALSE(batch, result, "Hard_Link into non-existent subdir fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Hard_Link into non-existent subdir sets Err_error");
    TEST_TRUE(batch, Folder_Exists(folder, &foo_boffo),
              "Entry still exists at old path");

    Err_set_error(NULL);
    result = Folder_Rename(folder, &nope_nyet, &boffo);
    TEST_FALSE(batch, result, "Hard_Link non-existent source file fails");
    TEST_TRUE(batch, Err_get_error() != NULL,
              "Hard_Link non-existent source file sets Err_error");

    Folder_Delete(folder, &foo_bar);
    Folder_Delete(folder, &foo_boffo);
    Folder_Delete(folder, &foo);
    DECREF(folder);
    tear_down();
}

static void
test_Close(TestBatch *batch, set_up_t set_up, tear_down_t tear_down) {
    Folder *folder = set_up();
    Folder_Close(folder);
    PASS(batch, "Close() concludes without incident");
    Folder_Close(folder);
    Folder_Close(folder);
    PASS(batch, "Calling Close() multiple times is safe");
    DECREF(folder);
    tear_down();
}

uint32_t
TestFolderCommon_num_tests() {
    return 99;
}

void
TestFolderCommon_run_tests(void *test_batch, set_up_t set_up,
                           tear_down_t tear_down) {
    TestBatch *batch = (TestBatch*)test_batch;

    test_Local_Exists(batch, set_up, tear_down);
    test_Local_Is_Directory(batch, set_up, tear_down);
    test_Local_Find_Folder(batch, set_up, tear_down);
    test_Local_MkDir(batch, set_up, tear_down);
    test_Local_Open_Dir(batch, set_up, tear_down);
    test_Local_Open_FileHandle(batch, set_up, tear_down);
    test_Local_Delete(batch, set_up, tear_down);
    test_Rename(batch, set_up, tear_down);
    test_Hard_Link(batch, set_up, tear_down);
    test_Close(batch, set_up, tear_down);
}


