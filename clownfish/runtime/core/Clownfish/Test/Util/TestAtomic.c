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

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test.h"
#include "Clownfish/Test/Util/TestAtomic.h"
#include "Clownfish/Util/Atomic.h"

static void
test_cas_ptr(TestBatch *batch) {
    int    foo = 1;
    int    bar = 2;
    int   *foo_pointer = &foo;
    int   *bar_pointer = &bar;
    int   *target      = NULL;

    TEST_TRUE(batch,
              Atomic_cas_ptr((void**)&target, NULL, foo_pointer),
              "cas_ptr returns true on success");
    TEST_TRUE(batch, target == foo_pointer, "cas_ptr sets target");

    target = NULL;
    TEST_FALSE(batch,
               Atomic_cas_ptr((void**)&target, bar_pointer, foo_pointer),
               "cas_ptr returns false when it old_value doesn't match");
    TEST_TRUE(batch, target == NULL,
              "cas_ptr doesn't do anything to target when old_value doesn't match");

    target = foo_pointer;
    TEST_TRUE(batch,
              Atomic_cas_ptr((void**)&target, foo_pointer, bar_pointer),
              "cas_ptr from one value to another");
    TEST_TRUE(batch, target == bar_pointer, "cas_ptr sets target");
}

void
TestAtomic_run_tests() {
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);

    test_cas_ptr(batch);

    DECREF(batch);
}


