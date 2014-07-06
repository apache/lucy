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

#define C_TESTLUCY_TESTSEG
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestSegment.h"
#include "Lucy/Index/Segment.h"
#include "Lucy/Store/RAMFolder.h"

TestSegment*
TestSeg_new() {
    return (TestSegment*)Class_Make_Obj(TESTSEGMENT);
}

static void
test_fields(TestBatchRunner *runner) {
    Segment *segment = Seg_new(1);
    StackString *foo = SSTR_WRAP_UTF8("foo", 3);
    StackString *bar = SSTR_WRAP_UTF8("bar", 3);
    StackString *baz = SSTR_WRAP_UTF8("baz", 3);
    int32_t field_num;

    field_num = Seg_Add_Field(segment, (String*)foo);
    TEST_TRUE(runner, field_num == 1,
              "Add_Field returns field number, and field numbers start at 1");
    field_num = Seg_Add_Field(segment, (String*)bar);
    TEST_TRUE(runner, field_num == 2, "add a second field");
    field_num = Seg_Add_Field(segment, (String*)foo);
    TEST_TRUE(runner, field_num == 1,
              "Add_Field returns existing field number if field is already known");

    TEST_TRUE(runner, SStr_Equals(bar, (Obj*)Seg_Field_Name(segment, 2)),
              "Field_Name");
    TEST_TRUE(runner, Seg_Field_Name(segment, 3) == NULL,
              "Field_Name returns NULL for unknown field number");
    TEST_TRUE(runner, Seg_Field_Num(segment, (String*)bar) == 2,
              "Field_Num");
    TEST_TRUE(runner, Seg_Field_Num(segment, (String*)baz) == 0,
              "Field_Num returns 0 for unknown field name");

    DECREF(segment);
}

static void
test_metadata_storage(TestBatchRunner *runner) {
    Segment *segment = Seg_new(1);
    String *got;

    Seg_Store_Metadata_Utf8(segment, "foo", 3, (Obj*)Str_newf("bar"));
    got = (String*)Seg_Fetch_Metadata_Utf8(segment, "foo", 3);
    TEST_TRUE(runner,
              got
              && Str_Is_A(got, STRING)
              && Str_Equals_Utf8(got, "bar", 3),
              "metadata round trip"
             );
    DECREF(segment);
}

static void
test_seg_name_and_num(TestBatchRunner *runner) {
    Segment *segment_z = Seg_new(35);
    String *seg_z_name = Seg_num_to_name(35);
    TEST_TRUE(runner, Seg_Get_Number(segment_z) == INT64_C(35), "Get_Number");
    TEST_TRUE(runner, Str_Equals_Utf8(Seg_Get_Name(segment_z), "seg_z", 5),
              "Get_Name");
    TEST_TRUE(runner, Str_Equals_Utf8(seg_z_name, "seg_z", 5),
              "num_to_name");
    DECREF(seg_z_name);
    DECREF(segment_z);
}

static void
test_count(TestBatchRunner *runner) {
    Segment *segment = Seg_new(100);

    TEST_TRUE(runner, Seg_Get_Count(segment) == 0, "count starts off at 0");
    Seg_Set_Count(segment, 120);
    TEST_TRUE(runner, Seg_Get_Count(segment) == 120, "Set_Count");
    TEST_TRUE(runner, Seg_Increment_Count(segment, 10) == 130,
              "Increment_Count");

    DECREF(segment);
}

static void
test_Compare_To(TestBatchRunner *runner) {
    Segment *segment_1      = Seg_new(1);
    Segment *segment_2      = Seg_new(2);
    Segment *also_segment_2 = Seg_new(2);

    TEST_TRUE(runner, Seg_Compare_To(segment_1, (Obj*)segment_2) < 0,
              "Compare_To 1 < 2");
    TEST_TRUE(runner, Seg_Compare_To(segment_2, (Obj*)segment_1) > 0,
              "Compare_To 1 < 2");
    TEST_TRUE(runner, Seg_Compare_To(segment_1, (Obj*)segment_1) == 0,
              "Compare_To identity");
    TEST_TRUE(runner, Seg_Compare_To(segment_2, (Obj*)also_segment_2) == 0,
              "Compare_To 2 == 2");

    DECREF(segment_1);
    DECREF(segment_2);
    DECREF(also_segment_2);
}

static void
test_Write_File_and_Read_File(TestBatchRunner *runner) {
    RAMFolder *folder  = RAMFolder_new(NULL);
    Segment   *segment = Seg_new(100);
    Segment   *got     = Seg_new(100);
    String    *meta;
    String    *flotsam = (String*)SSTR_WRAP_UTF8("flotsam", 7);
    String    *jetsam  = (String*)SSTR_WRAP_UTF8("jetsam", 6);

    Seg_Set_Count(segment, 111);
    Seg_Store_Metadata_Utf8(segment, "foo", 3, (Obj*)Str_newf("bar"));
    Seg_Add_Field(segment, flotsam);
    Seg_Add_Field(segment, jetsam);

    RAMFolder_MkDir(folder, Seg_Get_Name(segment));
    Seg_Write_File(segment, (Folder*)folder);
    Seg_Read_File(got, (Folder*)folder);

    TEST_TRUE(runner, Seg_Get_Count(got) == Seg_Get_Count(segment),
              "Round-trip count through file");
    TEST_TRUE(runner,
              Seg_Field_Num(got, jetsam) == Seg_Field_Num(segment, jetsam),
              "Round trip field names through file");
    meta = (String*)Seg_Fetch_Metadata_Utf8(got, "foo", 3);
    TEST_TRUE(runner,
              meta
              && Str_Is_A(meta, STRING)
              && Str_Equals_Utf8(meta, "bar", 3),
              "Round trip metadata through file");

    DECREF(got);
    DECREF(segment);
    DECREF(folder);
}

void
TestSeg_Run_IMP(TestSegment *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 21);
    test_fields(runner);
    test_metadata_storage(runner);
    test_seg_name_and_num(runner);
    test_count(runner);
    test_Compare_To(runner);
    test_Write_File_and_Read_File(runner);
}


