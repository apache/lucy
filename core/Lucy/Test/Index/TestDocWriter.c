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

#define C_LUCY_TESTDOCWRITER
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestDocWriter.h"
#include "Lucy/Index/DocWriter.h"

TestDocWriter*
TestDocWriter_new() {
    TestDocWriter *self = (TestDocWriter*)VTable_Make_Obj(TESTDOCWRITER);
    return TestDocWriter_init(self);
}

TestDocWriter*
TestDocWriter_init(TestDocWriter *self) {
    return (TestDocWriter*)TestBatch_init((TestBatch*)self, 1);
}

void
TestDocWriter_run_tests(TestDocWriter *self) {
    TestBatch *batch = (TestBatch*)self;
    PASS(batch, "placeholder");
}


