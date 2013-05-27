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

#define C_TESTLUCY_TESTSEGWRITER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/Test/TestFormatter.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestSegWriter.h"
#include "Lucy/Index/SegWriter.h"

TestSegWriter*
TestSegWriter_new(TestFormatter *formatter) {
    TestSegWriter *self = (TestSegWriter*)VTable_Make_Obj(TESTSEGWRITER);
    return TestSegWriter_init(self, formatter);
}

TestSegWriter*
TestSegWriter_init(TestSegWriter *self, TestFormatter *formatter) {
    return (TestSegWriter*)TestBatch_init((TestBatch*)self, 1, formatter);
}

void
TestSegWriter_run_tests(TestSegWriter *self) {
    TestBatch *batch = (TestBatch*)self;
    PASS(batch, "placeholder");
}


