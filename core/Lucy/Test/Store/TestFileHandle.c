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

#define C_TESTLUCY_TESTINSTREAM
#define C_LUCY_INSTREAM
#define C_LUCY_FILEWINDOW
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Store/TestFileHandle.h"
#include "Lucy/Store/FileHandle.h"
#include "Lucy/Store/FileWindow.h"

TestFileHandle*
TestFH_new() {
    return (TestFileHandle*)Class_Make_Obj(TESTFILEHANDLE);
}

static void
S_no_op_method(const void *vself) {
    UNUSED_VAR(vself);
}

static FileHandle*
S_new_filehandle() {
    StackString *class_name = SSTR_WRAP_UTF8("TestFileHandle", 14);
    FileHandle *fh;
    Class *klass = Class_fetch_class((String*)class_name);
    if (!klass) {
        klass = Class_singleton((String*)class_name, FILEHANDLE);
    }
    Class_Override(klass, S_no_op_method, LUCY_FH_Close_OFFSET);
    fh = (FileHandle*)Class_Make_Obj(klass);
    return FH_do_open(fh, NULL, 0);
}

void
TestFH_Run_IMP(TestFileHandle *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 2);

    FileHandle    *fh    = S_new_filehandle();
    StackString *foo   = SSTR_WRAP_UTF8("foo", 3);

    TEST_TRUE(runner, Str_Equals_Utf8(FH_Get_Path(fh), "", 0), "Get_Path");
    FH_Set_Path(fh, (String*)foo);
    TEST_TRUE(runner, Str_Equals(FH_Get_Path(fh), (Obj*)foo), "Set_Path");

    DECREF(fh);
}


