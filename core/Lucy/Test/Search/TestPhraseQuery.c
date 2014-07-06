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

#define C_TESTLUCY_TESTPHRASEQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestPhraseQuery.h"
#include "Lucy/Search/PhraseQuery.h"
#include "Lucy/Util/Freezer.h"

TestPhraseQuery*
TestPhraseQuery_new() {
    return (TestPhraseQuery*)Class_Make_Obj(TESTPHRASEQUERY);
}

static void
test_Dump_And_Load(TestBatchRunner *runner) {
    PhraseQuery *query
        = TestUtils_make_phrase_query("content", "a", "b", "c", NULL);
    Obj         *dump  = (Obj*)PhraseQuery_Dump(query);
    PhraseQuery *twin = (PhraseQuery*)Freezer_load(dump);
    TEST_TRUE(runner, PhraseQuery_Equals(query, (Obj*)twin),
              "Dump => Load round trip");
    DECREF(query);
    DECREF(dump);
    DECREF(twin);
}

void
TestPhraseQuery_Run_IMP(TestPhraseQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1);
    test_Dump_And_Load(runner);
}


