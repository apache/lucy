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

#define LUCY_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "Clownfish/Test.h"
#include "Clownfish/Test/TestUtils.h"
#include "Clownfish/Test/TestLockFreeRegistry.h"
#include "Clownfish/LockFreeRegistry.h"

StupidHashCharBuf*
StupidHashCharBuf_new(const char *text) {
    return (StupidHashCharBuf*)CB_new_from_utf8(text, strlen(text));
}

int32_t
StupidHashCharBuf_hash_sum(StupidHashCharBuf *self) {
    UNUSED_VAR(self);
    return 1;
}

static void
test_all(TestBatch *batch) {
    LockFreeRegistry *registry = LFReg_new(10);
    StupidHashCharBuf *foo = StupidHashCharBuf_new("foo");
    StupidHashCharBuf *bar = StupidHashCharBuf_new("bar");
    StupidHashCharBuf *baz = StupidHashCharBuf_new("baz");
    StupidHashCharBuf *foo_dupe = StupidHashCharBuf_new("foo");

    TEST_TRUE(batch, LFReg_Register(registry, (Obj*)foo, (Obj*)foo),
              "Register() returns true on success");
    TEST_FALSE(batch,
               LFReg_Register(registry, (Obj*)foo_dupe, (Obj*)foo_dupe),
               "Can't Register() keys that test equal");

    TEST_TRUE(batch, LFReg_Register(registry, (Obj*)bar, (Obj*)bar),
              "Register() key with the same Hash_Sum but that isn't Equal");

    TEST_TRUE(batch, LFReg_Fetch(registry, (Obj*)foo_dupe) == (Obj*)foo,
              "Fetch()");
    TEST_TRUE(batch, LFReg_Fetch(registry, (Obj*)bar) == (Obj*)bar,
              "Fetch() again");
    TEST_TRUE(batch, LFReg_Fetch(registry, (Obj*)baz) == NULL,
              "Fetch() non-existent key returns NULL");

    DECREF(foo_dupe);
    DECREF(baz);
    DECREF(bar);
    DECREF(foo);
    DECREF(registry);
}

void
TestLFReg_run_tests() {
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);
    test_all(batch);

    DECREF(batch);
}


