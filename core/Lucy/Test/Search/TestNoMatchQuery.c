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

#define C_LUCY_TESTNOMATCHQUERY
#include "Lucy/Util/ToolSet.h"
#include <math.h>

#include "Lucy/Test.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Test/Search/TestNoMatchQuery.h"
#include "Lucy/Search/NoMatchQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch) {
    NoMatchQuery *query = NoMatchQuery_new();
    Obj          *dump  = (Obj*)NoMatchQuery_Dump(query);
    NoMatchQuery *clone = (NoMatchQuery*)NoMatchQuery_Load(query, dump);

    TEST_TRUE(batch, NoMatchQuery_Equals(query, (Obj*)clone),
              "Dump => Load round trip");
    TEST_FALSE(batch, NoMatchQuery_Equals(query, (Obj*)&EMPTY), "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}


void
TestNoMatchQuery_run_tests() {
    TestBatch *batch = TestBatch_new(2);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


