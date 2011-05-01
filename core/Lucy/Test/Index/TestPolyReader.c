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

#define C_LUCY_TESTPOLYREADER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestPolyReader.h"
#include "Lucy/Index/PolyReader.h"

static void
test_sub_tick(TestBatch *batch) {
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
    TEST_INT_EQ(batch, i, num_segs, "got all sub_tick() calls right");
    DECREF(offsets);
    FREEMEM(ints);
}

void
TestPolyReader_run_tests() {
    TestBatch *batch = TestBatch_new(1);
    TestBatch_Plan(batch);

    test_sub_tick(batch);

    DECREF(batch);
}

