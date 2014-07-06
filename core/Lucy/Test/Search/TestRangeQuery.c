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

#define C_TESTLUCY_TESTRANGEQUERY
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestRangeQuery.h"
#include "Lucy/Search/RangeQuery.h"

TestRangeQuery*
TestRangeQuery_new() {
    return (TestRangeQuery*)Class_Make_Obj(TESTRANGEQUERY);
}

static void
test_Dump_Load_and_Equals(TestBatchRunner *runner) {
    RangeQuery *query 
        = TestUtils_make_range_query("content", "foo", "phooey", true, true);
    RangeQuery *lo_term_differs 
        = TestUtils_make_range_query("content", "goo", "phooey", true, true);
    RangeQuery *hi_term_differs 
        = TestUtils_make_range_query("content", "foo", "gooey", true, true);
    RangeQuery *include_lower_differs 
        = TestUtils_make_range_query("content", "foo", "phooey", false, true);
    RangeQuery *include_upper_differs 
        = TestUtils_make_range_query("content", "foo", "phooey", true, false);
    Obj        *dump  = (Obj*)RangeQuery_Dump(query);
    RangeQuery *clone = (RangeQuery*)RangeQuery_Load(lo_term_differs, dump);

    TEST_FALSE(runner, RangeQuery_Equals(query, (Obj*)lo_term_differs),
               "Equals() false with different lower term");
    TEST_FALSE(runner, RangeQuery_Equals(query, (Obj*)hi_term_differs),
               "Equals() false with different upper term");
    TEST_FALSE(runner, RangeQuery_Equals(query, (Obj*)include_lower_differs),
               "Equals() false with different include_lower");
    TEST_FALSE(runner, RangeQuery_Equals(query, (Obj*)include_upper_differs),
               "Equals() false with different include_upper");
    TEST_TRUE(runner, RangeQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");

    DECREF(query);
    DECREF(lo_term_differs);
    DECREF(hi_term_differs);
    DECREF(include_lower_differs);
    DECREF(include_upper_differs);
    DECREF(dump);
    DECREF(clone);
}


void
TestRangeQuery_Run_IMP(TestRangeQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 5);
    test_Dump_Load_and_Equals(runner);
}


