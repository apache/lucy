#define C_LUCY_CHARBUF
#include "Lucy/Util/ToolSet.h"

#define CHAZ_USE_SHORT_NAMES
#include "Charmonizer/Test.h"

/* rmdir */
#ifdef CHY_HAS_DIRECT_H
  #include <direct.h>
#endif

/* rmdir */
#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h>
#endif

#include "Lucy/Test/Store/TestFSDirHandle.h"
#include "Lucy/Store/FSDirHandle.h"
#include "Lucy/Store/FSFolder.h"
#include "Lucy/Store/OutStream.h"

static CharBuf foo           = ZCB_LITERAL("foo");
static CharBuf boffo         = ZCB_LITERAL("boffo");
static CharBuf foo_boffo     = ZCB_LITERAL("foo/boffo");
static CharBuf test_dir      = ZCB_LITERAL("_fsdir_test");

static void
test_all(TestBatch *batch)
{
    FSFolder     *folder = FSFolder_new(&test_dir);
    OutStream    *outstream;
    FSDirHandle  *dh;
    CharBuf      *entry;
    bool_t        saw_foo       = false;
    bool_t        saw_boffo     = false;
    bool_t        foo_was_dir   = false;
    bool_t        boffo_was_dir = false; 
    int           count         = 0;

    rmdir("_fsdir_test");
    FSFolder_Initialize(folder);
    FSFolder_MkDir(folder, &foo);
    outstream = FSFolder_Open_Out(folder, &boffo);
    DECREF(outstream);
    outstream = FSFolder_Open_Out(folder, &foo_boffo);
    DECREF(outstream);

    dh = FSDH_open(&test_dir);
    entry = FSDH_Get_Entry(dh);
    while (FSDH_Next(dh)) {
        count++;
        if (CB_Equals(entry, (Obj*)&foo)) { 
            saw_foo = true;
            foo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
        else if (CB_Equals(entry, (Obj*)&boffo)) {
            saw_boffo = true;
            boffo_was_dir = FSDH_Entry_Is_Dir(dh);
        }
    }
    ASSERT_INT_EQ(batch, 2, count, "correct number of entries");
    ASSERT_TRUE(batch, saw_foo, "Directory was iterated over");
    ASSERT_TRUE(batch, foo_was_dir, 
        "Dir correctly identified by Entry_Is_Dir");
    ASSERT_TRUE(batch, saw_boffo, "File was iterated over");
    ASSERT_FALSE(batch, boffo_was_dir, 
        "File correctly identified by Entry_Is_Dir");

    DECREF(dh);
    Folder_Delete(folder, &foo_boffo);
    Folder_Delete(folder, &foo);
    Folder_Delete(folder, &boffo);
    DECREF(folder);
    rmdir("_fsdir_test");
}

void
TestFSDH_run_tests()
{
    TestBatch *batch = Test_new_batch("TestFSDirHandle", 5, NULL);

    PLAN(batch);
    test_all(batch);

    batch->destroy(batch);
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

