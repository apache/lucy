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

#define C_TESTLUCY_TESTNOTQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestNOTQuery.h"
#include "Lucy/Search/NOTQuery.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Util/Freezer.h"

TestNOTQuery*
TestNOTQuery_new() {
    return (TestNOTQuery*)Class_Make_Obj(TESTNOTQUERY);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    Query    *a_leaf        = (Query*)TestUtils_make_leaf_query(NULL, "a");
    Query    *b_leaf        = (Query*)TestUtils_make_leaf_query(NULL, "b");
    NOTQuery *query         = NOTQuery_new(a_leaf);
    NOTQuery *kids_differ   = NOTQuery_new(b_leaf);
    NOTQuery *boost_differs = NOTQuery_new(a_leaf);
    Obj      *dump          = (Obj*)NOTQuery_Dump(query);
    NOTQuery *clone         = (NOTQuery*)Freezer_load(dump);

    TEST_FALSE(runner, NOTQuery_Equals(query, (Obj*)kids_differ),
               "Different kids spoil Equals");
    TEST_TRUE(runner, NOTQuery_Equals(query, (Obj*)boost_differs),
              "Equals with identical boosts");
    NOTQuery_Set_Boost(boost_differs, 1.5);
    TEST_FALSE(runner, NOTQuery_Equals(query, (Obj*)boost_differs),
               "Different boost spoils Equals");
    TEST_TRUE(runner, NOTQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(a_leaf);
    DECREF(b_leaf);
    DECREF(query);
    DECREF(kids_differ);
    DECREF(boost_differs);
    DECREF(dump);
    DECREF(clone);
}

void
TestNOTQuery_Run_IMP(TestNOTQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 4);
    test_Dump_Load_and_Equals(runner);
}


