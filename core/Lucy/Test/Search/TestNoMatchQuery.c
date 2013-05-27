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

#include "Clownfish/Test/TestFormatter.h"
#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestNoMatchQuery.h"
#include "Lucy/Search/NoMatchQuery.h"

TestNoMatchQuery*
TestNoMatchQuery_new(TestFormatter *formatter) {
    TestNoMatchQuery *self = (TestNoMatchQuery*)VTable_Make_Obj(TESTNOMATCHQUERY);
    return TestNoMatchQuery_init(self, formatter);
}

TestNoMatchQuery*
TestNoMatchQuery_init(TestNoMatchQuery *self, TestFormatter *formatter) {
    return (TestNoMatchQuery*)TestBatch_init((TestBatch*)self, 2, formatter);
}

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    NoMatchQuery *query = NoMatchQuery_new();
    Obj          *dump  = (Obj*)NoMatchQuery_Dump(query);
    NoMatchQuery *clone = (NoMatchQuery*)NoMatchQuery_Load(query, dump);

    TEST_TRUE(batch, NoMatchQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");
    TEST_FALSE(batch, NoMatchQuery_Equals(query, (Obj*)CFISH_TRUE),
               "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}


void
TestNoMatchQuery_run_tests(TestNoMatchQuery *self) {
    TestBatch *batch = (TestBatch*)self;
    test_Dump_Load_and_Equals(batch);
}


