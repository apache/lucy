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

#define C_LUCY_TESTFORMATTERTAP
#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test/Formatter/TestFormatterTAP.h"
#include "Lucy/Test.h"
#include "Clownfish/Test/TestRunner.h"
#include "Clownfish/VTable.h"

TestFormatterTAP*
TestFormatterTAP_new() {
    TestFormatterTAP *self
        = (TestFormatterTAP*)VTable_Make_Obj(TESTFORMATTERTAP);
    return TestFormatterTAP_init(self);
}

TestFormatterTAP*
TestFormatterTAP_init(TestFormatterTAP *self) {
    return (TestFormatterTAP*)TestFormatter_init((TestFormatter*)self);
}

void
TestFormatterTAP_batch_prologue(TestFormatterTAP *self, TestBatch *batch) {
    UNUSED_VAR(self);
    printf("1..%" PRId64 "\n", TestBatch_Get_Num_Planned(batch));
}

void
TestFormatterTAP_vtest_result(TestFormatterTAP *self, bool pass,
                              uint32_t test_num, const char *fmt,
                              va_list args) {
    UNUSED_VAR(self);
    const char *result = pass ? "ok" : "not ok";
    printf("%s %d - ", result, test_num);
    vprintf(fmt, args);
    printf("\n");
}

void
TestFormatterTAP_vtest_comment(TestFormatterTAP *self, const char *fmt,
                               va_list args) {
    UNUSED_VAR(self);
    printf("#   ");
    vprintf(fmt, args);
}

void
TestFormatterTAP_vbatch_comment(TestFormatterTAP *self, const char *fmt,
                                va_list args) {
    UNUSED_VAR(self);
    printf("# ");
    vprintf(fmt, args);
}

void
TestFormatterTAP_summary(TestFormatterTAP *self, TestRunner *runner) {
    UNUSED_VAR(self);
    UNUSED_VAR(runner);
}


