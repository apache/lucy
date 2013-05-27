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

#define C_TESTLUCY_TESTHIGHLIGHTWRITER
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestFormatter.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Index/TestHighlightWriter.h"
#include "Lucy/Index/HighlightWriter.h"

TestHighlightWriter*
TestHLWriter_new(TestFormatter *formatter) {
    TestHighlightWriter *self = (TestHighlightWriter*)VTable_Make_Obj(TESTHIGHLIGHTWRITER);
    return TestHLWriter_init(self, formatter);
}

TestHighlightWriter*
TestHLWriter_init(TestHighlightWriter *self, TestFormatter *formatter) {
    return (TestHighlightWriter*)TestBatch_init((TestBatch*)self, 1, formatter);
}

void
TestHLWriter_run_tests(TestHighlightWriter *self) {
    TestBatch *batch = (TestBatch*)self;
    PASS(batch, "Placeholder");
}


