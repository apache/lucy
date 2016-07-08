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

#define C_TESTLUCY_TESTTERMINFO
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Search/TestSpan.h"
#include "Lucy/Search/Span.h"

TestSpan*
TestSpan_new() {
    return (TestSpan*)Class_Make_Obj(TESTSPAN);
}

void 
test_span_init_values(TestBatchRunner *runner) {
    Span* span = Span_new(2,3,7.0);
    TEST_INT_EQ(runner, Span_Get_Offset(span), 2, "get_offset" );
    TEST_INT_EQ(runner, Span_Get_Length(span), 3, "get_length" );
    TEST_FLOAT_EQ(runner, Span_Get_Weight(span), 7.0, "get_weight" );

    Span_Set_Offset(span, 10);
    Span_Set_Length(span, 1);
    Span_Set_Weight(span, 4.0);

    TEST_INT_EQ(runner, Span_Get_Offset(span), 10, "set_offset" );
    TEST_INT_EQ(runner, Span_Get_Length(span), 1, "set_length" );
    TEST_FLOAT_EQ(runner, Span_Get_Weight(span), 4.0, "set_weight" );

    DECREF(span);
}

void
TestSpan_Run_IMP(TestSpan *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 6);
    test_span_init_values(runner);
}
