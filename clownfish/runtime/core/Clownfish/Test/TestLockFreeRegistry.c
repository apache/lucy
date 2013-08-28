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

#include <string.h>

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "charmony.h"

#include "Clownfish/Test/TestLockFreeRegistry.h"

#include "Clownfish/LockFreeRegistry.h"
#include "Clownfish/Test.h"
#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Clownfish/VTable.h"

TestLockFreeRegistry*
TestLFReg_new() {
    return (TestLockFreeRegistry*)VTable_Make_Obj(TESTLOCKFREEREGISTRY);
}

StupidHashCharBuf*
StupidHashCharBuf_new(const char *text) {
    return (StupidHashCharBuf*)Str_new_from_utf8(text, strlen(text));
}

int32_t
StupidHashCharBuf_Hash_Sum_IMP(StupidHashCharBuf *self) {
    UNUSED_VAR(self);
    return 1;
}

static void
test_all(TestBatchRunner *runner) {
    LockFreeRegistry *registry = LFReg_new(10);
    StupidHashCharBuf *foo = StupidHashCharBuf_new("foo");
    StupidHashCharBuf *bar = StupidHashCharBuf_new("bar");
    StupidHashCharBuf *baz = StupidHashCharBuf_new("baz");
    StupidHashCharBuf *foo_dupe = StupidHashCharBuf_new("foo");

    TEST_TRUE(runner, LFReg_Register(registry, (Obj*)foo, (Obj*)foo),
              "Register() returns true on success");
    TEST_FALSE(runner,
               LFReg_Register(registry, (Obj*)foo_dupe, (Obj*)foo_dupe),
               "Can't Register() keys that test equal");

    TEST_TRUE(runner, LFReg_Register(registry, (Obj*)bar, (Obj*)bar),
              "Register() key with the same Hash_Sum but that isn't Equal");

    TEST_TRUE(runner, LFReg_Fetch(registry, (Obj*)foo_dupe) == (Obj*)foo,
              "Fetch()");
    TEST_TRUE(runner, LFReg_Fetch(registry, (Obj*)bar) == (Obj*)bar,
              "Fetch() again");
    TEST_TRUE(runner, LFReg_Fetch(registry, (Obj*)baz) == NULL,
              "Fetch() non-existent key returns NULL");

    DECREF(foo_dupe);
    DECREF(baz);
    DECREF(bar);
    DECREF(foo);
    DECREF(registry);
}

void
TestLFReg_Run_IMP(TestLockFreeRegistry *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 6);
    test_all(runner);
}


