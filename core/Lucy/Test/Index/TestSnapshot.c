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

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestSnapshot.h"
#include "Lucy/Index/Snapshot.h"
#include "Lucy/Store/RAMFolder.h"

TestSnapshot*
TestSnapshot_new() {
    return (TestSnapshot*)Class_Make_Obj(TESTSNAPSHOT);
}

static void
test_Add_and_Delete(TestBatchRunner *runner) {
    Snapshot *snapshot = Snapshot_new();
    String *foo = (String*)SSTR_WRAP_UTF8("foo", 3);
    String *bar = (String*)SSTR_WRAP_UTF8("bar", 3);

    Snapshot_Add_Entry(snapshot, foo);
    Snapshot_Add_Entry(snapshot, foo); // redundant
    VArray *entries = Snapshot_List(snapshot);
    TEST_INT_EQ(runner, Snapshot_Num_Entries(snapshot), 1,
                "One entry added");
    TEST_TRUE(runner, Str_Equals(foo, VA_Fetch(entries, 0)), "correct entry");
    DECREF(entries);

    Snapshot_Add_Entry(snapshot, bar);
    TEST_INT_EQ(runner, Snapshot_Num_Entries(snapshot), 2,
                "second entry added");
    Snapshot_Delete_Entry(snapshot, foo);
    TEST_INT_EQ(runner, Snapshot_Num_Entries(snapshot), 1, "Delete_Entry");

    DECREF(snapshot);
}

static void
test_path_handling(TestBatchRunner *runner) {
    Snapshot *snapshot = Snapshot_new();
    Folder   *folder   = (Folder*)RAMFolder_new(NULL);
    String   *snap     = (String*)SSTR_WRAP_UTF8("snap", 4);
    String   *crackle  = (String*)SSTR_WRAP_UTF8("crackle", 7);

    Snapshot_Write_File(snapshot, folder, snap);
    TEST_TRUE(runner, Str_Equals(snap, (Obj*)Snapshot_Get_Path(snapshot)),
              "Write_File() sets path as a side effect");

    Folder_Rename(folder, snap, crackle);
    Snapshot_Read_File(snapshot, folder, crackle);
    TEST_TRUE(runner, Str_Equals(crackle, (Obj*)Snapshot_Get_Path(snapshot)),
              "Read_File() sets path as a side effect");

    Snapshot_Set_Path(snapshot, snap);
    TEST_TRUE(runner, Str_Equals(snap, (Obj*)Snapshot_Get_Path(snapshot)),
              "Set_Path()");

    DECREF(folder);
    DECREF(snapshot);
}

static void
test_Read_File_and_Write_File(TestBatchRunner *runner) {
    Snapshot *snapshot = Snapshot_new();
    Folder   *folder   = (Folder*)RAMFolder_new(NULL);
    String   *snap     = (String*)SSTR_WRAP_UTF8("snap", 4);
    String   *foo      = (String*)SSTR_WRAP_UTF8("foo", 3);

    Snapshot_Add_Entry(snapshot, foo);
    Snapshot_Write_File(snapshot, folder, snap);

    Snapshot *dupe = Snapshot_new();
    Snapshot *read_retval = Snapshot_Read_File(dupe, folder, snap);
    TEST_TRUE(runner, dupe == read_retval, "Read_File() returns the object");

    VArray *orig_list = Snapshot_List(snapshot);
    VArray *dupe_list = Snapshot_List(dupe);
    TEST_TRUE(runner, VA_Equals(orig_list, (Obj*)dupe_list),
              "Round trip through Write_File() and Read_File()");

    DECREF(orig_list);
    DECREF(dupe_list);
    DECREF(dupe);
    DECREF(snapshot);
    DECREF(folder);
}

void
TestSnapshot_Run_IMP(TestSnapshot *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 9);
    test_Add_and_Delete(runner);
    test_path_handling(runner);
    test_Read_File_and_Write_File(runner);
}


