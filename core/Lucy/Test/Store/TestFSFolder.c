#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

/* mkdir, rmdir */
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

/* rmdir */
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

/* mkdir, stat */
#ifdef CHY_HAS_SYS_STAT_H
  #include <sys/stat.h>
#endif

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSFolder.h"
#include "Lucy/Test/Store/TestFolderCommon.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/OutStream.h"

static CharBuf test_dir_name = ZCB_LITERAL("_fsfolder_test");
static CharBuf foo           = ZCB_LITERAL("foo");
static CharBuf bar           = ZCB_LITERAL("bar");
static CharBuf foo_boffo     = ZCB_LITERAL("foo/boffo");

static Folder*
S_set_up()
{
    FSFolder *folder = FSFolder_new(&test_dir_name);
    rmdir("_fsfolder_test");
    FSFolder_Initialize(folder);
    if (!FSFolder_Check(folder)) {
        RETHROW(INCREF(Err_get_error()));
    }
    return (Folder*)folder;
}

static void
S_tear_down()
{
    struct stat stat_buf;
    rmdir("_fsfolder_test");
    if (stat("_fsfolder_test", &stat_buf) != -1) {
        THROW(ERR, "Can't clean up directory _fsfolder_test");
    }
}

static void
test_Initialize_and_Check(TestBatch *batch)
{
    FSFolder *folder = FSFolder_new(&test_dir_name);
    rmdir("_fsfolder_test");
    ASSERT_FALSE(batch, FSFolder_Check(folder), 
        "Check() returns false when folder dir doesn't exist");
    FSFolder_Initialize(folder);
    PASS(batch, "Initialize() concludes without incident");
    ASSERT_TRUE(batch, FSFolder_Check(folder), 
        "Initialize() created dir, and now Check() succeeds");
    DECREF(folder);
    S_tear_down();
}

static void
test_protect_symlinks(TestBatch *batch) 
{
#ifdef CHY_HAS_UNISTD_H
    FSFolder *folder = (FSFolder*)S_set_up();
    
    FSFolder_MkDir(folder, &foo);
    FSFolder_MkDir(folder, &bar);
    OutStream *outstream = FSFolder_Open_Out(folder, &foo_boffo);
    DECREF(outstream);

    if (symlink("_fsfolder_test/foo/boffo", "_fsfolder_test/bar/banana")) {
        FAIL(batch, "symlink() failed");
        FAIL(batch, "symlink() failed");
        FAIL(batch, "symlink() failed");
        FAIL(batch, "symlink() failed");
    }
    else {
        ASSERT_TRUE(batch, FSFolder_Delete_Tree(folder, &bar), 
            "Delete_Tree() returns true"), 
        ASSERT_FALSE(batch, FSFolder_Exists(folder, &bar), 
            "Tree is really gone");
        ASSERT_TRUE(batch, FSFolder_Exists(folder, &foo),
            "Original folder sill there");
        ASSERT_TRUE(batch, FSFolder_Exists(folder, &foo_boffo),
            "Delete_Tree() did not follow directory symlink");
        FSFolder_Delete_Tree(folder, &foo);
    }
    DECREF(folder);
    S_tear_down();
#else
    /* TODO: Add test for Windows. */
    SKIP(batch, "No symlink() function");
    SKIP(batch, "No symlink() function");
    SKIP(batch, "No symlink() function");
    SKIP(batch, "No symlink() function");
#endif /* CHY_HAS_UNISTD_H */
}

void
TestFSFolder_run_tests()
{
    u32_t num_tests = TestFolderCommon_num_tests() + 7;
    TestBatch *batch = TestBatch_new(num_tests);

    TestBatch_Plan(batch);
    test_Initialize_and_Check(batch);
    TestFolderCommon_run_tests(batch, S_set_up, S_tear_down);
    test_protect_symlinks(batch);

    DECREF(batch);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

