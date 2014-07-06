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

#define C_TESTLUCY_TESTPOLYREADER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestPolyReader.h"
#include "Lucy/Index/PolyReader.h"

TestPolyReader*
TestPolyReader_new() {
    return (TestPolyReader*)Class_Make_Obj(TESTPOLYREADER);
}

static void
test_sub_tick(TestBatchRunner *runner) {
    size_t num_segs = 255;
    int32_t *ints = (int32_t*)MALLOCATE(num_segs * sizeof(int32_t));
    size_t i;
    for (i = 0; i < num_segs; i++) {
        ints[i] = i;
    }
    I32Array *offsets = I32Arr_new(ints, num_segs);
    for (i = 1; i < num_segs; i++) {
        if (PolyReader_sub_tick(offsets, i) != i - 1) { break; }
    }
    TEST_INT_EQ(runner, i, num_segs, "got all sub_tick() calls right");
    DECREF(offsets);
    FREEMEM(ints);
}

void
TestPolyReader_Run_IMP(TestPolyReader *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1);
    test_sub_tick(runner);
}

