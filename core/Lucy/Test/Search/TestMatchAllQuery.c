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

#define C_TESTLUCY_TESTMATCHALLQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/TestHarness/TestFormatter.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestMatchAllQuery.h"
#include "Lucy/Search/MatchAllQuery.h"

TestMatchAllQuery*
TestMatchAllQuery_new(TestFormatter *formatter) {
    TestMatchAllQuery *self = (TestMatchAllQuery*)VTable_Make_Obj(TESTMATCHALLQUERY);
    return TestMatchAllQuery_init(self, formatter);
}

TestMatchAllQuery*
TestMatchAllQuery_init(TestMatchAllQuery *self, TestFormatter *formatter) {
    return (TestMatchAllQuery*)TestBatch_init((TestBatch*)self, 2, formatter);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    MatchAllQuery *query = MatchAllQuery_new();
    Obj           *dump  = (Obj*)MatchAllQuery_Dump(query);
    MatchAllQuery *clone = (MatchAllQuery*)MatchAllQuery_Load(query, dump);

    TEST_TRUE(batch, MatchAllQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");
    TEST_FALSE(batch, MatchAllQuery_Equals(query, (Obj*)CFISH_TRUE),
               "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}


void
TestMatchAllQuery_run_tests(TestMatchAllQuery *self) {
    TestBatch *batch = (TestBatch*)self;
    test_Dump_Load_and_Equals(batch);
}


