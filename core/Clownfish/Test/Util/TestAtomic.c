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

#include "Clownfish/Test/Util/TestAtomic.h"

#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/Util/Atomic.h"
#include "Clownfish/VTable.h"

TestAtomic*
TestAtomic_new() {
    return (TestAtomic*)VTable_Make_Obj(TESTATOMIC);
}

static void
test_cas_ptr(TestBatchRunner *runner) {
    int    foo = 1;
    int    bar = 2;
    int   *foo_pointer = &foo;
    int   *bar_pointer = &bar;
    int   *target      = NULL;

    TEST_TRUE(runner,
              Atomic_cas_ptr((void**)&target, NULL, foo_pointer),
              "cas_ptr returns true on success");
    TEST_TRUE(runner, target == foo_pointer, "cas_ptr sets target");

    target = NULL;
    TEST_FALSE(runner,
               Atomic_cas_ptr((void**)&target, bar_pointer, foo_pointer),
               "cas_ptr returns false when it old_value doesn't match");
    TEST_TRUE(runner, target == NULL,
              "cas_ptr doesn't do anything to target when old_value doesn't match");

    target = foo_pointer;
    TEST_TRUE(runner,
              Atomic_cas_ptr((void**)&target, foo_pointer, bar_pointer),
              "cas_ptr from one value to another");
    TEST_TRUE(runner, target == bar_pointer, "cas_ptr sets target");
}

void
TestAtomic_run(TestAtomic *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 6);
    test_cas_ptr(runner);
}


