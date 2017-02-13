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

#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test/Search/MockSearcher.h"
#include "Lucy/Test/TestUtils.h"
#include "Lucy/Util/Freezer.h"
#include "LucyX/Search/ProximityQuery.h"
#include "LucyX/Test/Search/TestProximityQuery.h"

TestProximityQuery*
TestProximityQuery_new() {
    return (TestProximityQuery*)Class_Make_Obj(TESTPROXIMITYQUERY);
}

static ProximityQuery*
S_make_proximity_query() {
    Vector *terms = Vec_new(2);
    Vec_Push(terms, (Obj*)Str_newf("a"));
    Vec_Push(terms, (Obj*)Str_newf("b"));
    ProximityQuery *query
        = ProximityQuery_new(SSTR_WRAP_C("content"), terms, 4);
    DECREF(terms);
    return query;
}

static void
test_Dump_And_Load(TestBatchRunner *runner) {
    ProximityQuery *query = S_make_proximity_query();
    Obj *dump = (Obj*)ProximityQuery_Dump(query);
    ProximityQuery *twin = (ProximityQuery*)Freezer_load(dump);
    TEST_TRUE(runner, ProximityQuery_Equals(query, (Obj*)twin),
              "Dump => Load round trip");
    DECREF(query);
    DECREF(dump);
    DECREF(twin);
}

static void
test_freeze_thaw_compiler(TestBatchRunner *runner) {
    ProximityQuery *query = S_make_proximity_query();
    Searcher *searcher = (Searcher*)MockSearcher_new();
    ProximityCompiler *compiler
        = ProximityCompiler_new(query, searcher, 880.0f);
    TestUtils_test_freeze_thaw(runner, (Obj*)compiler, "compiler");
    DECREF(searcher);
    DECREF(query);
}

void
TestProximityQuery_Run_IMP(TestProximityQuery *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 2);
    test_Dump_And_Load(runner);
    test_freeze_thaw_compiler(runner);
}


