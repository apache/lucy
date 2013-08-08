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
#define TESTCFISH_USE_SHORT_NAMES

#include "Clownfish/Test/TestErr.h"

#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/VTable.h"

TestErr*
TestErr_new() {
    return (TestErr*)VTable_Make_Obj(TESTERR);
}

static void
test_To_String(TestBatchRunner *runner) {
    CharBuf *message = CB_newf("oops");
    Err *error = Err_new(message);
    CharBuf *string = Err_To_String(error);
    TEST_TRUE(runner, CB_Equals(message, (Obj*)string),
              "Stringifies as message");
    DECREF(string);
    DECREF(error);
}

void
TestErr_Run_IMP(TestErr *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 1);
    test_To_String(runner);
}


