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

#define C_CFISH_TESTRUNNER
#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test/TestRunner.h"
#include "Clownfish/Err.h"
#include "Clownfish/Test/TestBatch.h"
#include "Clownfish/Test/TestFormatter.h"
#include "Clownfish/VTable.h"

TestRunner*
TestRunner_new(TestFormatter *formatter) {
    TestRunner *self = (TestRunner*)VTable_Make_Obj(TESTRUNNER);
    return TestRunner_init(self, formatter);
}

TestRunner*
TestRunner_init(TestRunner *self, TestFormatter *formatter) {
    self->formatter          = (TestFormatter*)INCREF(formatter);
    self->num_tests          = 0;
    self->num_tests_failed   = 0;
    self->num_batches        = 0;
    self->num_batches_failed = 0;

    return self;
}

void
TestRunner_destroy(TestRunner *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTRUNNER);
}

bool
TestRunner_run_batch(TestRunner *self, TestBatch *batch) {
    bool success = TestBatch_Run(batch);

    self->num_tests        += TestBatch_Get_Num_Tests(batch);
    self->num_tests_failed += TestBatch_Get_Num_Failed(batch);
    self->num_batches      += 1;

    if (!success) {
        self->num_batches_failed += 1;
    }

    return success;
}

bool
TestRunner_finish(TestRunner *self) {
    TestFormatter_Summary(self->formatter, self);

    return self->num_batches != 0 && self->num_batches_failed == 0;
}

uint32_t
TestRunner_get_num_tests(TestRunner *self) {
    return self->num_tests;
}

uint32_t
TestRunner_get_num_tests_failed(TestRunner *self) {
    return self->num_tests_failed;
}

uint32_t
TestRunner_get_num_batches(TestRunner *self) {
    return self->num_batches;
}

uint32_t
TestRunner_get_num_batches_failed(TestRunner *self) {
    return self->num_batches_failed;
}


