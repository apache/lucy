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

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test.h"
#include "Clownfish/Test/Util/TestMemory.h"
#include "Clownfish/Util/Memory.h"

static void
test_oversize__growth_rate(TestBatch *batch) {
    bool     success             = true;
    uint64_t size                = 0;
    double   growth_count        = 0;
    double   average_growth_rate = 0.0;

    while (size < SIZE_MAX) {
        uint64_t next_size = Memory_oversize((size_t)size + 1, sizeof(void*));
        if (next_size < size) {
            success = false;
            FAIL(batch, "Asked for %" PRId64 ", got smaller amount %" PRId64,
                 size + 1, next_size);
            break;
        }
        if (size > 0) {
            growth_count += 1;
            double growth_rate = U64_TO_DOUBLE(next_size) /
                                 U64_TO_DOUBLE(size);
            double sum = growth_rate + (growth_count - 1) * average_growth_rate;
            average_growth_rate = sum / growth_count;
            if (average_growth_rate < 1.1) {
                FAIL(batch, "Average growth rate dropped below 1.1x: %f",
                     average_growth_rate);
                success = false;
                break;
            }
        }
        size = next_size;
    }
    TEST_TRUE(batch, growth_count > 0, "Grew %f times", growth_count);
    if (success) {
        TEST_TRUE(batch, average_growth_rate > 1.1,
                  "Growth rate of oversize() averages above 1.1: %.3f",
                  average_growth_rate);
    }

    for (int minimum = 1; minimum < 8; minimum++) {
        uint64_t next_size = Memory_oversize(minimum, sizeof(void*));
        double growth_rate = U64_TO_DOUBLE(next_size) / (double)minimum;
        TEST_TRUE(batch, growth_rate > 1.2,
                  "Growth rate is higher for smaller arrays (%d, %.3f)", minimum,
                  growth_rate);
    }
}

static void
test_oversize__ceiling(TestBatch *batch) {
    for (int width = 0; width < 10; width++) {
        size_t size = Memory_oversize(SIZE_MAX, width);
        TEST_TRUE(batch, size == SIZE_MAX,
                  "Memory_oversize hits ceiling at SIZE_MAX (width %d)", width);
        size = Memory_oversize(SIZE_MAX - 1, width);
        TEST_TRUE(batch, size == SIZE_MAX,
                  "Memory_oversize hits ceiling at SIZE_MAX (width %d)", width);
    }
}

static void
test_oversize__rounding(TestBatch *batch) {
    int widths[] = { 1, 2, 4, 0 };

    for (int width_tick = 0; widths[width_tick] != 0; width_tick++) {
        int width = widths[width_tick];
        for (int i = 0; i < 25; i++) {
            size_t size = Memory_oversize(i, width);
            size_t bytes = size * width;
            if (bytes % sizeof(void*) != 0) {
                FAIL(batch, "Rounding failure for %d, width %d",
                     i, width);
                return;
            }
        }
    }
    PASS(batch, "Round allocations up to the size of a pointer");
}

void
TestMemory_run_tests() {
    TestBatch *batch = TestBatch_new(30);

    TestBatch_Plan(batch);
    test_oversize__growth_rate(batch);
    test_oversize__ceiling(batch);
    test_oversize__rounding(batch);

    DECREF(batch);
}


