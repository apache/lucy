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

#include <math.h>
#include <stdio.h>
#include <string.h>

#define C_CFISH_TESTBATCHRUNNER
#define CFISH_USE_SHORT_NAMES

#include "Clownfish/TestHarness/TestBatchRunner.h"

#include "Clownfish/String.h"
#include "Clownfish/Err.h"
#include "Clownfish/TestHarness/TestBatch.h"
#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

struct try_run_tests_context {
    TestBatchRunner *runner;
    TestBatch       *batch;
};

static void
S_try_run_tests(void *context);

static bool
S_vtest_true(TestBatchRunner *self, bool condition, const char *pattern,
             va_list args);

TestBatchRunner*
TestBatchRunner_new(TestFormatter *formatter) {
    TestBatchRunner *self = (TestBatchRunner*)VTable_Make_Obj(TESTBATCHRUNNER);
    return TestBatchRunner_init(self, formatter);
}

TestBatchRunner*
TestBatchRunner_init(TestBatchRunner *self, TestFormatter *formatter) {
    // Assign.
    self->formatter   = (TestFormatter*)INCREF(formatter);

    // Initialize.
    self->num_planned = 0;
    self->test_num    = 0;
    self->num_passed  = 0;
    self->num_failed  = 0;
    self->num_skipped = 0;

    return self;
}

void
TestBatchRunner_Destroy_IMP(TestBatchRunner *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTBATCHRUNNER);
}

bool
TestBatchRunner_Run_Batch_IMP(TestBatchRunner *self, TestBatch *batch) {
    struct try_run_tests_context args;
    args.runner = self;
    args.batch  = batch;
    Err *err = Err_trap(S_try_run_tests, &args);

    bool failed = false;
    if (err) {
        failed = true;
        String *mess = Err_Get_Mess(err);
        Err_warn_mess((String*)INCREF(mess));
    }
    if (self->num_failed > 0) {
        failed = true;
        TestFormatter_batch_comment(self->formatter, "%d/%d tests failed.\n",
                                    self->num_failed, self->test_num);
    }
    if (self->test_num != self->num_planned) {
        failed = true;
        TestFormatter_batch_comment(self->formatter,
                                    "Bad plan: You planned %d tests but ran"
                                    " %d.\n",
                                    self->num_planned, self->test_num);
    }

    return !failed;
}

static void
S_try_run_tests(void *context) {
    struct try_run_tests_context *args
        = (struct try_run_tests_context*)context;
    TestBatch_Run(args->batch, args->runner);
}

void
TestBatchRunner_Plan_IMP(TestBatchRunner *self, TestBatch *batch,
                         uint32_t num_planned) {
    self->num_planned = num_planned;
    TestFormatter_Batch_Prologue(self->formatter, batch, num_planned);
}

uint32_t
TestBatchRunner_Get_Num_Planned_IMP(TestBatchRunner *self) {
    return self->num_planned;
}

uint32_t
TestBatchRunner_Get_Num_Tests_IMP(TestBatchRunner *self) {
    return self->test_num;
}

uint32_t
TestBatchRunner_Get_Num_Failed_IMP(TestBatchRunner *self) {
    return self->num_failed;
}

bool
TestBatchRunner_test_true(TestBatchRunner *self, bool condition,
                          const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VTest_True(self, condition, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_test_false(TestBatchRunner *self, bool condition,
                           const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VTest_False(self, condition, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_test_int_equals(TestBatchRunner *self, long got, long expected,
                                const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VTest_Int_Equals(self, got, expected,
                                                   pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_test_float_equals(TestBatchRunner *self, double got,
                                  double expected, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VTest_Float_Equals(self, got, expected,
                                                     pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_test_string_equals(TestBatchRunner *self, const char *got,
                                   const char *expected, const char *pattern,
                                   ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VTest_String_Equals(self, got, expected,
                                                      pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_pass(TestBatchRunner *self, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VPass(self, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatchRunner_fail(TestBatchRunner *self, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatchRunner_VFail(self, pattern, args);
    va_end(args);
    return result;
}

void
TestBatchRunner_skip(TestBatchRunner *self, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    TestBatchRunner_VSkip(self, pattern, args);
    va_end(args);
}

bool
TestBatchRunner_VTest_True_IMP(TestBatchRunner *self, bool condition,
                               const char *pattern, va_list args) {
    return S_vtest_true(self, condition, pattern, args);
}

bool
TestBatchRunner_VTest_False_IMP(TestBatchRunner *self, bool condition,
                                const char *pattern, va_list args) {
    return S_vtest_true(self, !condition, pattern, args);
}

bool
TestBatchRunner_VTest_Int_Equals_IMP(TestBatchRunner *self, long got,
                                     long expected, const char *pattern,
                                     va_list args) {
    bool pass = (got == expected);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%ld', got '%ld'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatchRunner_VTest_Float_Equals_IMP(TestBatchRunner *self, double got,
                                       double expected, const char *pattern,
                                       va_list args) {
    double relative_error = got / expected - 1.0;
    bool   pass           = (fabs(relative_error) < 1e-6);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%e', got '%e'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatchRunner_VTest_String_Equals_IMP(TestBatchRunner *self, const char *got,
                                        const char *expected,
                                        const char *pattern, va_list args) {
    bool pass = (strcmp(got, expected) == 0);
    S_vtest_true(self, pass, pattern, args);
    if (!pass) {
        TestFormatter_test_comment(self->formatter,
                                   "Expected '%s', got '%s'.\n",
                                   expected, got);
    }
    return pass;
}

bool
TestBatchRunner_VPass_IMP(TestBatchRunner *self, const char *pattern,
                          va_list args) {
    return S_vtest_true(self, true, pattern, args);
}

bool
TestBatchRunner_VFail_IMP(TestBatchRunner *self, const char *pattern,
                          va_list args) {
    return S_vtest_true(self, false, pattern, args);
}

void
TestBatchRunner_VSkip_IMP(TestBatchRunner *self, const char *pattern,
                          va_list args) {
    self->test_num++;
    // TODO: Add a VTest_Skip method to TestFormatter
    TestFormatter_VTest_Result(self->formatter, true, self->test_num,
                               pattern, args);
    self->num_skipped++;
}

static bool
S_vtest_true(TestBatchRunner* self, bool condition, const char *pattern,
             va_list args) {
    // Increment test number.
    self->test_num++;

    if (condition) {
        self->num_passed++;
    }
    else {
        self->num_failed++;
    }

    TestFormatter_VTest_Result(self->formatter, condition, self->test_num,
                               pattern, args);

    return condition;
}


