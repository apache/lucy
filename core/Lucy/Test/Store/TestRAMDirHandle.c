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
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestRAMDirHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/RAMFolder.h"
#include "Lucy/Store/RAMDirHandle.h"

static void
test_all(TestBatch *batch) {
    RAMFolder *folder        = RAMFolder_new(NULL);
    CharBuf   *foo           = (CharBuf*)ZCB_WRAP_STR("foo", 3);
    CharBuf   *boffo         = (CharBuf*)ZCB_WRAP_STR("boffo", 5);
    CharBuf   *foo_boffo     = (CharBuf*)ZCB_WRAP_STR("foo/boffo", 9);
    bool_t     saw_foo       = false;
    bool_t     saw_boffo     = false;
    bool_t     foo_was_dir   = false;
    bool_t     boffo_was_dir = false;
    int        count         = 0;

    // Set up folder contents.
    RAMFolder_MkDir(folder, foo);
    FileHandle *fh = RAMFolder_Open_FileHandle(folder, boffo,
                                               FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);
    fh = RAMFolder_Open_FileHandle(folder, foo_boffo,
                                   FH_CREATE | FH_WRITE_ONLY);
    DECREF(fh);

    RAMDirHandle *dh    = RAMDH_new(folder);
    CharBuf      *entry = RAMDH_Get_Entry(dh);
    while (RAMDH_Next(dh)) {
        count++;
        if (CB_Equals(entry, (Obj*)foo)) {
            saw_foo = true;
            foo_was_dir = RAMDH_Entry_Is_Dir(dh);
        }
        else if (CB_Equals(entry, (Obj*)boffo)) {
            saw_boffo = true;
            boffo_was_dir = RAMDH_Entry_Is_Dir(dh);
        }
    }
    TEST_INT_EQ(batch, 2, count, "correct number of entries");
    TEST_TRUE(batch, saw_foo, "Directory was iterated over");
    TEST_TRUE(batch, foo_was_dir,
              "Dir correctly identified by Entry_Is_Dir");
    TEST_TRUE(batch, saw_boffo, "File was iterated over");
    TEST_FALSE(batch, boffo_was_dir,
               "File correctly identified by Entry_Is_Dir");

    {
        uint32_t refcount = RAMFolder_Get_RefCount(folder);
        RAMDH_Close(dh);
        TEST_INT_EQ(batch, RAMFolder_Get_RefCount(folder), refcount - 1,
                    "Folder reference released by Close()");
    }

    DECREF(dh);
    DECREF(folder);
}

void
TestRAMDH_run_tests() {
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);
    test_all(batch);

    DECREF(batch);
}


