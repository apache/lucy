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
#include <stdlib.h>
#include <string.h>

#define C_LUCY_TESTBATCH
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Clownfish/Test/Formatter/TestFormatterTAP.h"
#include "Clownfish/Test/TestFormatter.h"
#include "Clownfish/Test/TestRunner.h"

static bool
S_vtest_true(TestBatch *self, bool condition, const char *pattern,
             va_list args);

static VArray*
S_all_test_batches() {
    VArray *batches = VA_new(0);

    return batches;
}

bool
Test_run_batch(CharBuf *class_name, TestFormatter *formatter) {
    VArray   *batches = S_all_test_batches();
    uint32_t  size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);

        if (CB_Equals(TestBatch_Get_Class_Name(batch), (Obj*)class_name)) {
            TestRunner *runner  = TestRunner_new(formatter);
            bool result = TestRunner_Run_Batch(runner, batch);
            DECREF(runner);
            DECREF(batches);
            return result;
        }
    }

    DECREF(batches);
    THROW(ERR, "Couldn't find test class '%o'", class_name);
    UNREACHABLE_RETURN(bool);
}

bool
Test_run_all_batches(TestFormatter *formatter) {
    TestRunner *runner  = TestRunner_new(formatter);
    VArray     *batches = S_all_test_batches();
    uint32_t    size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);
        TestRunner_Run_Batch(runner, batch);
    }

    bool result = TestRunner_Finish(runner);

    DECREF(runner);
    DECREF(batches);
    return result;
}

TestBatch*
TestBatch_new(int64_t num_tests) {
    TestBatch *self = (TestBatch*)VTable_Make_Obj(TESTBATCH);
    return TestBatch_init(self, num_tests);
}

TestBatch*
TestBatch_init(TestBatch *self, int64_t num_tests) {
    // Assign.
    self->num_tests       = num_tests;

    // Initialize.
    self->formatter       = (TestFormatter*)TestFormatterTAP_new();
    self->test_num        = 0;
    self->num_passed      = 0;
    self->num_failed      = 0;
    self->num_skipped     = 0;

    // Unbuffer stdout. TODO: move this elsewhere.
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }

    return self;
}

void
TestBatch_destroy(TestBatch *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTBATCH);
}

void
TestBatch_plan(TestBatch *self) {
    TestFormatter_Batch_Prologue(self->formatter, self);
}

void
TestBatch_run(TestBatch *self) {
}

int64_t
TestBatch_get_num_planned(TestBatch *self) {
    return self->num_tests;
}

int64_t
TestBatch_get_num_tests(TestBatch *self) {
    return self->test_num;
}

int64_t
TestBatch_get_num_failed(TestBatch *self) {
    return self->num_failed;
}

bool
TestBatch_test_true(void *vself, bool condition, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_True((TestBatch*)vself, condition,
                                         pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_false(void *vself, bool condition, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_False((TestBatch*)vself, condition,
                                          pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_int_equals(void *vself, long got, long expected,
                          const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_Int_Equals((TestBatch*)vself, got,
                                               expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_float_equals(void *vself, double got, double expected,
                            const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_Float_Equals((TestBatch*)vself, got,
                                                 expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_test_string_equals(void *vself, const char *got,
                             const char *expected, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VTest_String_Equals((TestBatch*)vself, got,
                                                  expected, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_pass(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VPass((TestBatch*)vself, pattern, args);
    va_end(args);
    return result;
}

bool
TestBatch_fail(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    bool result = TestBatch_VFail((TestBatch*)vself, pattern, args);
    va_end(args);
    return result;
}

void
TestBatch_skip(void *vself, const char *pattern, ...) {
    va_list args;
    va_start(args, pattern);
    TestBatch_VSkip((TestBatch*)vself, pattern, args);
    va_end(args);
}

bool
TestBatch_vtest_true(TestBatch *self, bool condition, const char *pattern,
                     va_list args) {
    return S_vtest_true(self, condition, pattern, args);
}

bool
TestBatch_vtest_false(TestBatch *self, bool condition,
                      const char *pattern, va_list args) {
    return S_vtest_true(self, !condition, pattern, args);
}

bool
TestBatch_vtest_int_equals(TestBatch *self, long got, long expected,
                           const char *pattern, va_list args) {
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
TestBatch_vtest_float_equals(TestBatch *self, double got, double expected,
                             const char *pattern, va_list args) {
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
TestBatch_vtest_string_equals(TestBatch *self, const char *got,
                              const char *expected, const char *pattern,
                              va_list args) {
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
TestBatch_vpass(TestBatch *self, const char *pattern, va_list args) {
    return S_vtest_true(self, true, pattern, args);
}

bool
TestBatch_vfail(TestBatch *self, const char *pattern, va_list args) {
    return S_vtest_true(self, false, pattern, args);
}

void
TestBatch_vskip(TestBatch *self, const char *pattern, va_list args) {
    self->test_num++;
    // TODO: Add a VTest_Skip method to TestFormatter
    TestFormatter_VTest_Result(self->formatter, true, self->num_tests,
                               pattern, args);
    self->num_skipped++;
}

static bool
S_vtest_true(TestBatch* self, bool condition, const char *pattern,
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


