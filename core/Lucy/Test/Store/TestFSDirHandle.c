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

// rmdir
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

// rmdir
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFSDirHandle.h"
#include "Lucy/Store/FSDirHandle.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/OutStream.h"

static void
test_all(TestBatch *batch) {
    CharBuf  *foo           = (CharBuf*)ZCB_WRAP_STR("foo", 3);
    CharBuf  *boffo         = (CharBuf*)ZCB_WRAP_STR("boffo", 5);
    CharBuf  *foo_boffo     = (CharBuf*)ZCB_WRAP_STR("foo/boffo", 9);
    CharBuf  *test_dir      = (CharBuf*)ZCB_WRAP_STR("_fsdir_test", 11);
    FSFolder *folder        = FSFolder_new(test_dir);
    bool_t    saw_foo       = false;
    bool_t    saw_boffo     = false;
    bool_t    foo_was_dir   = false;
    bool_t    boffo_was_dir = false;
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

    FSDirHandle  *dh    = FSDH_open(test_dir);
    CharBuf      *entry = FSDH_Get_Entry(dh);
    while (FSDH_Next(dh)) {
        count++;
        if (CB_Equals(entry, (Obj*)foo)) {
            saw_foo = true;
            foo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
        else if (CB_Equals(entry, (Obj*)boffo)) {
            saw_boffo = true;
            boffo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
    }
    TEST_INT_EQ(batch, 2, count, "correct number of entries");
    TEST_TRUE(batch, saw_foo, "Directory was iterated over");
    TEST_TRUE(batch, foo_was_dir,
              "Dir correctly identified by Entry_Is_Dir");
    TEST_TRUE(batch, saw_boffo, "File was iterated over");
    TEST_FALSE(batch, boffo_was_dir,
               "File correctly identified by Entry_Is_Dir");

    DECREF(dh);
    FSFolder_Delete(folder, foo_boffo);
    FSFolder_Delete(folder, foo);
    FSFolder_Delete(folder, boffo);
    DECREF(folder);
    rmdir("_fsdir_test");
}

void
TestFSDH_run_tests() {
    TestBatch *batch = TestBatch_new(5);

    TestBatch_Plan(batch);
    test_all(batch);

    DECREF(batch);
}


