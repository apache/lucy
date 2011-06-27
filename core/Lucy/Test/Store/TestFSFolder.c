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

#include "Lucy/Util/ToolSet.h"

// mkdir, rmdir
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

// rmdir
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

// mkdir, stat
#ifdef CHY_HAS_SYS_STAT_H
  #include <sys/stat.h>
#endif

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSFolder.h"
#include "Lucy/Test/Store/TestFolderCommon.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/OutStream.h"

/* The tests involving symlinks have to be run with administrator privileges
 * under Windows, so disable by default.
 */
#ifndef CHY_HAS_WINDOWS_H
#define ENABLE_SYMLINK_TESTS
// Create the symlinks needed by test_protect_symlinks().
static bool_t
S_create_test_symlinks(void);
#endif /* CHY_HAS_WINDOWS_H */

static Folder*
S_set_up() {
    rmdir("_fstest");
    CharBuf  *test_dir = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);
    FSFolder *folder = FSFolder_new(test_dir);
    FSFolder_Initialize(folder);
    if (!FSFolder_Check(folder)) {
        RETHROW(INCREF(Err_get_error()));
    }
    return (Folder*)folder;
}

static void
S_tear_down() {
    struct stat stat_buf;
    rmdir("_fstest");
    if (stat("_fstest", &stat_buf) != -1) {
        THROW(ERR, "Can't clean up directory _fstest");
    }
}

static void
test_Initialize_and_Check(TestBatch *batch) {
    rmdir("_fstest");
    CharBuf  *test_dir = (CharBuf*)ZCB_WRAP_STR("_fstest", 7);
    FSFolder *folder   = FSFolder_new(test_dir);
    TEST_FALSE(batch, FSFolder_Check(folder),
               "Check() returns false when folder dir doesn't exist");
    FSFolder_Initialize(folder);
    PASS(batch, "Initialize() concludes without incident");
    TEST_TRUE(batch, FSFolder_Check(folder),
              "Initialize() created dir, and now Check() succeeds");
    DECREF(folder);
    S_tear_down();
}

static void
test_protect_symlinks(TestBatch *batch) {
#ifdef ENABLE_SYMLINK_TESTS
    FSFolder *folder    = (FSFolder*)S_set_up();
    CharBuf  *foo       = (CharBuf*)ZCB_WRAP_STR("foo", 3);
    CharBuf  *bar       = (CharBuf*)ZCB_WRAP_STR("bar", 3);
    CharBuf  *foo_boffo = (CharBuf*)ZCB_WRAP_STR("foo/boffo", 9);

    FSFolder_MkDir(folder, foo);
    FSFolder_MkDir(folder, bar);
    OutStream *outstream = FSFolder_Open_Out(folder, foo_boffo);
    DECREF(outstream);

    if (!S_create_test_symlinks()) {
        FAIL(batch, "symlink creation failed");
        FAIL(batch, "symlink creation failed");
        FAIL(batch, "symlink creation failed");
        FAIL(batch, "symlink creation failed");
        FAIL(batch, "symlink creation failed");
        // Try to clean up anyway.
        FSFolder_Delete_Tree(folder, foo);
        FSFolder_Delete_Tree(folder, bar);
    }
    else {
        VArray *list = FSFolder_List_R(folder, NULL);
        bool_t saw_bazooka_boffo = false;
        for (uint32_t i = 0, max = VA_Get_Size(list); i < max; i++) {
            CharBuf *entry = (CharBuf*)VA_Fetch(list, i);
            if (CB_Ends_With_Str(entry, "bazooka/boffo", 13)) {
                saw_bazooka_boffo = true;
            }
        }
        TEST_FALSE(batch, saw_bazooka_boffo,
                   "List_R() shouldn't follow symlinks");
        DECREF(list);

        TEST_TRUE(batch, FSFolder_Delete_Tree(folder, bar),
                  "Delete_Tree() returns true");
        TEST_FALSE(batch, FSFolder_Exists(folder, bar),
                   "Tree is really gone");
        TEST_TRUE(batch, FSFolder_Exists(folder, foo),
                  "Original folder sill there");
        TEST_TRUE(batch, FSFolder_Exists(folder, foo_boffo),
                  "Delete_Tree() did not follow directory symlink");
        FSFolder_Delete_Tree(folder, foo);
    }
    DECREF(folder);
    S_tear_down();
#else
    SKIP(batch, "Tests requiring symlink() disabled");
    SKIP(batch, "Tests requiring symlink() disabled");
    SKIP(batch, "Tests requiring symlink() disabled");
    SKIP(batch, "Tests requiring symlink() disabled");
    SKIP(batch, "Tests requiring symlink() disabled");
#endif // ENABLE_SYMLINK_TESTS
}

void
test_disallow_updir(TestBatch *batch) {
    FSFolder *outer_folder = (FSFolder*)S_set_up();

    CharBuf *foo = (CharBuf*)ZCB_WRAP_STR("foo", 3);
    CharBuf *bar = (CharBuf*)ZCB_WRAP_STR("bar", 3);
    FSFolder_MkDir(outer_folder, foo);
    FSFolder_MkDir(outer_folder, bar);

    CharBuf *inner_path = (CharBuf*)ZCB_WRAP_STR("_fstest/foo", 11);
    FSFolder *foo_folder = FSFolder_new(inner_path);
    CharBuf *up_bar = (CharBuf*)ZCB_WRAP_STR("../bar", 6);
    TEST_FALSE(batch, FSFolder_Exists(foo_folder, up_bar),
               "up-dirs are inaccessible.");

    DECREF(foo_folder);
    FSFolder_Delete(outer_folder, foo);
    FSFolder_Delete(outer_folder, bar);
    DECREF(outer_folder);
    S_tear_down();
}

void
TestFSFolder_run_tests() {
    uint32_t num_tests = TestFolderCommon_num_tests() + 9;
    TestBatch *batch = TestBatch_new(num_tests);

    TestBatch_Plan(batch);
    test_Initialize_and_Check(batch);
    TestFolderCommon_run_tests(batch, S_set_up, S_tear_down);
    test_protect_symlinks(batch);
    test_disallow_updir(batch);

    DECREF(batch);
}

#ifdef ENABLE_SYMLINK_TESTS

#ifdef CHY_HAS_WINDOWS_H
#include "windows.h"
#elif defined(CHY_HAS_UNISTD_H)
#include <unistd.h>
#else
#error "Don't have either windows.h or unistd.h"
#endif

static bool_t
S_create_test_symlinks(void) {
#ifdef CHY_HAS_WINDOWS_H
    if (!CreateSymbolicLink("_fstest\\bar\\banana", "_fstest\\foo\\boffo", 0)
        || !CreateSymbolicLink("_fstest\\bar\\bazooka", "_fstest\\foo", 1)
       ) {
        return false;
    }
#else
    if (symlink("_fstest/foo/boffo", "_fstest/bar/banana")
        || symlink("_fstest/foo", "_fstest/bar/bazooka")
       ) {
        return false;
    }
#endif
    return true;
}

#endif /* ENABLE_SYMLINK_TESTS */

