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

#include <stdio.h>

#define C_CFISH_TESTSUITE
#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/TestHarness/TestSuite.h"

#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/TestHarness/TestBatch.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/TestHarness/TestSuiteRunner.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

static void
S_unbuffer_stdout();

TestSuite*
TestSuite_new() {
    TestSuite *self = (TestSuite*)VTable_Make_Obj(TESTSUITE);
    return TestSuite_init(self);
}

TestSuite*
TestSuite_init(TestSuite *self) {
    self->batches = VA_new(0);
    return self;
}

void
TestSuite_destroy(TestSuite *self) {
    DECREF(self->batches);
    SUPER_DESTROY(self, TESTSUITE);
}

void
TestSuite_add_batch(TestSuite *self, TestBatch *batch) {
    VA_Push(self->batches, (Obj*)batch);
}

bool
TestSuite_run_batch(TestSuite *self, CharBuf *class_name,
                    TestFormatter *formatter) {
    S_unbuffer_stdout();

    uint32_t size = VA_Get_Size(self->batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(self->batches, i);

        if (CB_Equals(TestBatch_Get_Class_Name(batch), (Obj*)class_name)) {
            TestBatchRunner *runner = TestBatchRunner_new(formatter);
            bool result = TestBatchRunner_Run_Batch(runner, batch);
            DECREF(runner);
            return result;
        }
    }

    THROW(ERR, "Couldn't find test class '%o'", class_name);
    UNREACHABLE_RETURN(bool);
}

bool
TestSuite_run_all_batches(TestSuite *self, TestFormatter *formatter) {
    S_unbuffer_stdout();

    TestSuiteRunner *runner = TestSuiteRunner_new(formatter);
    uint32_t size = VA_Get_Size(self->batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(self->batches, i);
        TestSuiteRunner_Run_Batch(runner, batch);
    }

    bool result = TestSuiteRunner_Finish(runner);

    DECREF(runner);
    return result;
}

static void
S_unbuffer_stdout() {
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }
}


