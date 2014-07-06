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

#define C_TESTLUCY_TESTPOLYQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestPolyQuery.h"
#include "Lucy/Search/ANDQuery.h"
#include "Lucy/Search/ORQuery.h"
#include "Lucy/Search/PolyQuery.h"
#include "Lucy/Search/LeafQuery.h"
#include "Lucy/Util/Freezer.h"

TestANDQuery*
TestANDQuery_new() {
    return (TestANDQuery*)Class_Make_Obj(TESTANDQUERY);
}

TestORQuery*
TestORQuery_new() {
    return (TestORQuery*)Class_Make_Obj(TESTORQUERY);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner, uint32_t boolop) {
    LeafQuery *a_leaf  = TestUtils_make_leaf_query(NULL, "a");
    LeafQuery *b_leaf  = TestUtils_make_leaf_query(NULL, "b");
    LeafQuery *c_leaf  = TestUtils_make_leaf_query(NULL, "c");
    PolyQuery *query
        = (PolyQuery*)TestUtils_make_poly_query(boolop, INCREF(a_leaf),
                                                INCREF(b_leaf), NULL);
    PolyQuery *kids_differ
        = (PolyQuery*)TestUtils_make_poly_query(boolop, INCREF(a_leaf),
                                                INCREF(b_leaf),
                                                INCREF(c_leaf),
                                                NULL);
    PolyQuery *boost_differs
        = (PolyQuery*)TestUtils_make_poly_query(boolop, INCREF(a_leaf),
                                                INCREF(b_leaf), NULL);
    Obj *dump = (Obj*)PolyQuery_Dump(query);
    PolyQuery *clone = (PolyQuery*)Freezer_load(dump);

    TEST_FALSE(runner, PolyQuery_Equals(query, (Obj*)kids_differ),
               "Different kids spoil Equals");
    TEST_TRUE(runner, PolyQuery_Equals(query, (Obj*)boost_differs),
              "Equals with identical boosts");
    PolyQuery_Set_Boost(boost_differs, 1.5);
    TEST_FALSE(runner, PolyQuery_Equals(query, (Obj*)boost_differs),
               "Different boost spoils Equals");
    TEST_TRUE(runner, PolyQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(a_leaf);
    DECREF(b_leaf);
    DECREF(c_leaf);
    DECREF(query);
    DECREF(kids_differ);
    DECREF(boost_differs);
    DECREF(dump);
    DECREF(clone);
}

void
TestANDQuery_Run_IMP(TestANDQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 4);
    test_Dump_Load_and_Equals(runner, BOOLOP_AND);
}

void
TestORQuery_Run_IMP(TestORQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 4);
    test_Dump_Load_and_Equals(runner, BOOLOP_OR);
}


