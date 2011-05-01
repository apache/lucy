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

#define C_LUCY_TESTI32ARRAY
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Object/TestI32Array.h"

static int32_t source_ints[] = { -1, 0, I32_MIN, I32_MAX, 1 };
static size_t num_ints = sizeof(source_ints) / sizeof(int32_t);

static void
test_all(TestBatch *batch) {
    I32Array *i32_array = I32Arr_new(source_ints, num_ints);
    int32_t  *ints_copy = (int32_t*)malloc(num_ints * sizeof(int32_t));
    I32Array *stolen    = I32Arr_new_steal(ints_copy, num_ints);
    size_t    num_matched;

    memcpy(ints_copy, source_ints, num_ints * sizeof(int32_t));

    TEST_TRUE(batch, I32Arr_Get_Size(i32_array) == num_ints,
              "Get_Size");
    TEST_TRUE(batch, I32Arr_Get_Size(stolen) == num_ints,
              "Get_Size for stolen");

    for (num_matched = 0; num_matched < num_ints; num_matched++) {
        if (source_ints[num_matched] != I32Arr_Get(i32_array, num_matched)) {
            break;
        }
    }
    TEST_INT_EQ(batch, num_matched, num_ints,
                "Matched all source ints with Get()");

    for (num_matched = 0; num_matched < num_ints; num_matched++) {
        if (source_ints[num_matched] != I32Arr_Get(stolen, num_matched)) {
            break;
        }
    }
    TEST_INT_EQ(batch, num_matched, num_ints,
                "Matched all source ints in stolen I32Array with Get()");

    DECREF(i32_array);
    DECREF(stolen);
}

void
TestI32Arr_run_tests() {
    TestBatch *batch = TestBatch_new(4);

    TestBatch_Plan(batch);
    test_all(batch);

    DECREF(batch);
}


