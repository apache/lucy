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

#define C_LUCY_TESTFORMATTER
#define LUCY_USE_SHORT_NAMES

#include "Clownfish/Test/TestFormatter.h"
#include "Clownfish/Err.h"

TestFormatter*
TestFormatter_init(TestFormatter *self) {
    ABSTRACT_CLASS_CHECK(self, TESTFORMATTER);
    return self;
}

void
TestFormatter_test_result(void *vself, bool pass, uint32_t test_num,
                          const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    TestFormatter_VTest_Result((TestFormatter*)vself, pass, test_num, fmt,
                               args);
    va_end(args);
}

void
TestFormatter_test_comment(void *vself, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    TestFormatter_VTest_Comment((TestFormatter*)vself, fmt, args);
    va_end(args);
}

void
TestFormatter_batch_comment(void *vself, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    TestFormatter_VBatch_Comment((TestFormatter*)vself, fmt, args);
    va_end(args);
}


