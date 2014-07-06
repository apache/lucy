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

#define C_TESTLUCY_TESTCASEFOLDER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Analysis/TestCaseFolder.h"
#include "Lucy/Analysis/CaseFolder.h"

TestCaseFolder*
TestCaseFolder_new() {
    return (TestCaseFolder*)Class_Make_Obj(TESTCASEFOLDER);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    CaseFolder *case_folder = CaseFolder_new();
    CaseFolder *other       = CaseFolder_new();
    Obj        *dump        = (Obj*)CaseFolder_Dump(case_folder);
    CaseFolder *clone       = (CaseFolder*)CaseFolder_Load(other, dump);

    TEST_TRUE(runner, CaseFolder_Equals(case_folder, (Obj*)other), "Equals");
    TEST_FALSE(runner, CaseFolder_Equals(case_folder, (Obj*)CFISH_TRUE),
               "Not Equals");
    TEST_TRUE(runner, CaseFolder_Equals(case_folder, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(case_folder);
    DECREF(other);
    DECREF(dump);
    DECREF(clone);
}

static void
test_analysis(TestBatchRunner *runner) {
    CaseFolder *case_folder = CaseFolder_new();
    String *source = Str_newf("caPiTal ofFensE");
    VArray *wanted = VA_new(1);
    VA_Push(wanted, (Obj*)Str_newf("capital offense"));
    TestUtils_test_analyzer(runner, (Analyzer*)case_folder, source, wanted,
                            "lowercase plain text");
    DECREF(wanted);
    DECREF(source);
    DECREF(case_folder);
}

void
TestCaseFolder_Run_IMP(TestCaseFolder *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 6);
    test_Dump_Load_and_Equals(runner);
    test_analysis(runner);
}



