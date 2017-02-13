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

#define C_TESTLUCY_TESTNOMATCHQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/Boolean.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestNoMatchQuery.h"
#include "Lucy/Search/NoMatchQuery.h"

TestNoMatchQuery*
TestNoMatchQuery_new() {
    return (TestNoMatchQuery*)Class_Make_Obj(TESTNOMATCHQUERY);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    NoMatchQuery *query = NoMatchQuery_new();
    Obj          *dump  = (Obj*)NoMatchQuery_Dump(query);
    NoMatchQuery *clone = (NoMatchQuery*)NoMatchQuery_Load(query, dump);

    TEST_TRUE(runner, NoMatchQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");
    TEST_FALSE(runner, NoMatchQuery_Equals(query, (Obj*)CFISH_TRUE),
               "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}

static void
test_freeze_thaw_compiler(TestBatchRunner *runner) {
    TestUtils_test_freeze_thaw(runner, (Obj*)NoMatchCompiler_new(),
                               "compiler");
}

void
TestNoMatchQuery_Run_IMP(TestNoMatchQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 3);
    test_Dump_Load_and_Equals(runner);
    test_freeze_thaw_compiler(runner);
}


