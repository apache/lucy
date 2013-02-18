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

#define C_LUCY_TESTBATCH
#define LUCY_USE_SHORT_NAMES
#include "Clownfish/Test/TestBatch.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Test/TestFormatter.h"
#include "Clownfish/VArray.h"
#include "Clownfish/VTable.h"

static bool
S_vtest_true(TestBatch *self, bool condition, const char *pattern,
             va_list args);

TestBatch*
TestBatch_new(uint32_t num_planned) {
    TestBatch *self = (TestBatch*)VTable_Make_Obj(TESTBATCH);
    return TestBatch_init(self, num_planned);
}

TestBatch*
TestBatch_init(TestBatch *self, uint32_t num_planned) {
    // Assign.
    self->num_planned     = num_planned;

    // Initialize.
    self->formatter       = (TestFormatter*)TestFormatterTAP_new();
    self->test_num        = 0;
    self->num_passed      = 0;
    self->num_failed      = 0;
    self->num_skipped     = 0;

    return self;
}

void
TestBatch_destroy(TestBatch *self) {
    DECREF(self->formatter);
    SUPER_DESTROY(self, TESTBATCH);
}

bool
TestBatch_run(TestBatch *self) {
    TestFormatter_Batch_Prologue(self->formatter, self);

    TestBatch_Run_Tests(self);

    bool failed = false;
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

uint32_t
TestBatch_get_num_planned(TestBatch *self) {
    return self->num_planned;
}

uint32_t
TestBatch_get_num_tests(TestBatch *self) {
    return self->test_num;
}

uint32_t
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
    TestFormatter_VTest_Result(self->formatter, true, self->test_num,
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


