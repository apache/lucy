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

#define C_CFISH_TESTSUITERUNNER
#define CFISH_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/TestHarness/TestSuiteRunner.h"

#include "Clownfish/Err.h"
#include "Clownfish/TestHarness/TestBatch.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/VTable.h"

TestSuiteRunner*
TestSuiteRunner_new(TestFormatter *formatter) {
    TestSuiteRunner *self = (TestSuiteRunner*)VTable_Make_Obj(TESTSUITERUNNER);
    return TestSuiteRunner_init(self, formatter);
}

TestSuiteRunner*
TestSuiteRunner_init(TestSuiteRunner *self, TestFormatter *formatter) {
    self->formatter          = (TestFormatter*)INCREF(formatter);
    self->num_tests          = 0;
    self->num_tests_failed   = 0;
    self->num_batches        = 0;
    self->num_batches_failed = 0;

    return self;
}

void
TestSuiteRunner_Destroy_IMP(TestSuiteRunner *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTSUITERUNNER);
}

bool
TestSuiteRunner_Run_Batch_IMP(TestSuiteRunner *self, TestBatch *batch) {
    TestBatchRunner *batch_runner = TestBatchRunner_new(self->formatter);
    bool success = TestBatchRunner_Run_Batch(batch_runner, batch);

    self->num_tests        += TestBatchRunner_Get_Num_Tests(batch_runner);
    self->num_tests_failed += TestBatchRunner_Get_Num_Failed(batch_runner);
    self->num_batches      += 1;

    if (!success) {
        self->num_batches_failed += 1;
    }

    DECREF(batch_runner);
    return success;
}

bool
TestSuiteRunner_Finish_IMP(TestSuiteRunner *self) {
    TestFormatter_Summary(self->formatter, self);

    return self->num_batches != 0 && self->num_batches_failed == 0;
}

uint32_t
TestSuiteRunner_Get_Num_Tests_IMP(TestSuiteRunner *self) {
    return self->num_tests;
}

uint32_t
TestSuiteRunner_Get_Num_Tests_Failed_IMP(TestSuiteRunner *self) {
    return self->num_tests_failed;
}

uint32_t
TestSuiteRunner_Get_Num_Batches_IMP(TestSuiteRunner *self) {
    return self->num_batches;
}

uint32_t
TestSuiteRunner_Get_Num_Batches_Failed_IMP(TestSuiteRunner *self) {
    return self->num_batches_failed;
}


