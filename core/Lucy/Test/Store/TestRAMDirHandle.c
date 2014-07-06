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
#include "Lucy/Test/Store/TestRAMDirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Store/RAMDirHandle.h"

TestRAMDirHandle*
TestRAMDH_new() {
    return (TestRAMDirHandle*)Class_Make_Obj(TESTRAMDIRHANDLE);
}

static void
test_all(TestBatchRunner *runner) {
    RAMFolder *folder        = RAMFolder_new(NULL);
    String    *foo           = (String*)SSTR_WRAP_UTF8("foo", 3);
    String    *boffo         = (String*)SSTR_WRAP_UTF8("boffo", 5);
    String    *foo_boffo     = (String*)SSTR_WRAP_UTF8("foo/boffo", 9);
    bool       saw_foo       = false;
    bool       saw_boffo     = false;
    bool       foo_was_dir   = false;
    bool       boffo_was_dir = false;
    int        count         = 0;

    // Set up folder contents.
    RAMFolder_MkDir(folder, foo);
    FileHandle *fh = RAMFolder_Open_FileHandle(folder, boffo,
                                               FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = RAMFolder_Open_FileHandle(folder, foo_boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    RAMDirHandle *dh = RAMDH_new(folder);
    while (RAMDH_Next(dh)) {
        count++;
        String *entry = RAMDH_Get_Entry(dh);
        if (Str_Equals(entry, (Obj*)foo)) {
            saw_foo = true;
            foo_was_dir = RAMDH_Entry_Is_Dir(dh);
        }
        else if (Str_Equals(entry, (Obj*)boffo)) {
            saw_boffo = true;
            boffo_was_dir = RAMDH_Entry_Is_Dir(dh);
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

    uint32_t refcount = RAMFolder_Get_RefCount(folder);
    RAMDH_Close(dh);
    TEST_INT_EQ(runner, RAMFolder_Get_RefCount(folder), refcount - 1,
                "Folder reference released by Close()");

    DECREF(dh);
    DECREF(folder);
}

void
TestRAMDH_Run_IMP(TestRAMDirHandle *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 6);
    test_all(runner);
}


