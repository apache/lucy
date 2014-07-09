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

#include "charmony.h"

// rmdir
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

// rmdir
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSDirHandle.h"
#include "Lucy/Store/FSDirHandle.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/OutStream.h"

TestFSDirHandle*
TestFSDH_new() {
    return (TestFSDirHandle*)Class_Make_Obj(TESTFSDIRHANDLE);
}

static void
test_all(TestBatchRunner *runner) {
    String   *foo           = (String*)SSTR_WRAP_UTF8("foo", 3);
    String   *boffo         = (String*)SSTR_WRAP_UTF8("boffo", 5);
    String   *foo_boffo     = (String*)SSTR_WRAP_UTF8("foo/boffo", 9);
    String   *test_dir      = (String*)SSTR_WRAP_UTF8("_fsdir_test", 11);
    FSFolder *folder        = FSFolder_new(test_dir);
    bool      saw_foo       = false;
    bool      saw_boffo     = false;
    bool      foo_was_dir   = false;
    bool      boffo_was_dir = false;
    int       count         = 0;

    // Clean up after previous failed runs.
    FSFolder_Delete(folder, foo_boffo);
    FSFolder_Delete(folder, foo);
    FSFolder_Delete(folder, boffo);
    rmdir("_fsdir_test");

    FSFolder_Initialize(folder);
    FSFolder_MkDir(folder, foo);
    OutStream *outstream = FSFolder_Open_Out(folder, boffo);
    DECREF(outstream);
    outstream = FSFolder_Open_Out(folder, foo_boffo);
    DECREF(outstream);

    FSDirHandle *dh = FSDH_open(test_dir);
    while (FSDH_Next(dh)) {
        count++;
        String *entry = FSDH_Get_Entry(dh);
        if (Str_Equals(entry, (Obj*)foo)) {
            saw_foo = true;
            foo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
        else if (Str_Equals(entry, (Obj*)boffo)) {
            saw_boffo = true;
            boffo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
        DECREF(entry);
    }
    TEST_INT_EQ(runner, 2, count, "correct number of entries");
    TEST_TRUE(runner, saw_foo, "Directory was iterated over");
    TEST_TRUE(runner, foo_was_dir,
              "Dir correctly identified by Entry_Is_Dir");
    TEST_TRUE(runner, saw_boffo, "File was iterated over");
    TEST_FALSE(runner, boffo_was_dir,
               "File correctly identified by Entry_Is_Dir");

    DECREF(dh);
    FSFolder_Delete(folder, foo_boffo);
    FSFolder_Delete(folder, foo);
    FSFolder_Delete(folder, boffo);
    DECREF(folder);
    rmdir("_fsdir_test");
}

void
TestFSDH_Run_IMP(TestFSDirHandle *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 5);
    test_all(runner);
}


