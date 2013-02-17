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

#define C_LUCY_TESTFORMATTERCF
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test/Formatter/TestFormatterCF.h"
#include "Clownfish/CharBuf.h"
#include "Lucy/Test.h"
#include "Clownfish/Test/TestRunner.h"
#include "Clownfish/VTable.h"

TestFormatterCF*
TestFormatterCF_new() {
    TestFormatterCF *self
        = (TestFormatterCF*)VTable_Make_Obj(TESTFORMATTERCF);
    return TestFormatterCF_init(self);
}

TestFormatterCF*
TestFormatterCF_init(TestFormatterCF *self) {
    return (TestFormatterCF*)TestFormatter_init((TestFormatter*)self);
}

void
TestFormatterCF_batch_prologue(TestFormatterCF *self, TestBatch *batch) {
    UNUSED_VAR(self);
    CharBuf *class_name = TestBatch_Get_Class_Name(batch);
    printf("Testing %s...\n", CB_Get_Ptr8(class_name));
}

void
TestFormatterCF_vtest_result(TestFormatterCF *self, bool pass,
                             uint32_t test_num, const char *fmt,
                             va_list args) {
    UNUSED_VAR(self);
    if (!pass) {
        printf("  Failed test %u: ", test_num);
        vprintf(fmt, args);
        printf("\n");
    }
}

void
TestFormatterCF_vtest_comment(TestFormatterCF *self, const char *fmt,
                              va_list args) {
    UNUSED_VAR(self);
    printf("    ");
    vprintf(fmt, args);
}

void
TestFormatterCF_vbatch_comment(TestFormatterCF *self, const char *fmt,
                               va_list args) {
    UNUSED_VAR(self);
    printf("  ");
    vprintf(fmt, args);
}

void
TestFormatterCF_summary(TestFormatterCF *self, TestRunner *runner) {
    UNUSED_VAR(self);
    uint32_t num_batches        = TestRunner_Get_Num_Batches(runner);
    uint32_t num_batches_failed = TestRunner_Get_Num_Batches_Failed(runner);
    uint32_t num_tests          = TestRunner_Get_Num_Tests(runner);
    uint32_t num_tests_failed   = TestRunner_Get_Num_Tests_Failed(runner);

    if (num_batches == 0) {
        printf("No tests planned or run.\n");
    }
    else if (num_batches_failed == 0) {
        printf("%u batches passed. %u tests passed.\n", num_batches,
               num_tests);
        printf("Result: PASS\n");
    }
    else {
        printf("%u/%u batches failed. %u/%u tests failed.\n",
               num_batches_failed, num_batches, num_tests_failed, num_tests);
        printf("Result: FAIL\n");
    }
}


