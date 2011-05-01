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

#define C_LUCY_TESTCASEFOLDER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Analysis/TestCaseFolder.h"
#include "Lucy/Analysis/CaseFolder.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    CaseFolder *case_folder = CaseFolder_new();
    CaseFolder *other       = CaseFolder_new();
    Obj        *dump        = (Obj*)CaseFolder_Dump(case_folder);
    CaseFolder *clone       = (CaseFolder*)CaseFolder_Load(other, dump);

    TEST_TRUE(batch, CaseFolder_Equals(case_folder, (Obj*)other), "Equals");
    TEST_FALSE(batch, CaseFolder_Equals(case_folder, (Obj*)&EMPTY), "Not Equals");
    TEST_TRUE(batch, CaseFolder_Equals(case_folder, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(case_folder);
    DECREF(other);
    DECREF(dump);
    DECREF(clone);
}

static void
test_analysis(TestBatch *batch) {
    CaseFolder *case_folder = CaseFolder_new();
    CharBuf *source = CB_newf("caPiTal ofFensE");
    VArray *wanted = VA_new(1);
    VA_Push(wanted, (Obj*)CB_newf("capital offense"));
    TestUtils_test_analyzer(batch, (Analyzer*)case_folder, source, wanted,
                            "lowercase plain text");
    DECREF(wanted);
    DECREF(source);
    DECREF(case_folder);
}

void
TestCaseFolder_run_tests() {
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);
    test_analysis(batch);

    DECREF(batch);
}



