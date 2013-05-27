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

#define CHY_USE_SHORT_NAMES
#define CFISH_USE_SHORT_NAMES
#define TESTCFISH_USE_SHORT_NAMES

#include "Clownfish/Test.h"

#include "Clownfish/Err.h"
#include "Clownfish/TestHarness/TestBatch.h"
#include "Clownfish/TestHarness/TestFormatter.h"
#include "Clownfish/TestHarness/TestRunner.h"
#include "Clownfish/VArray.h"

#include "Clownfish/Test/TestByteBuf.h"
#include "Clownfish/Test/TestCharBuf.h"
#include "Clownfish/Test/TestErr.h"
#include "Clownfish/Test/TestHash.h"
#include "Clownfish/Test/TestLockFreeRegistry.h"
#include "Clownfish/Test/TestNum.h"
#include "Clownfish/Test/TestObj.h"
#include "Clownfish/Test/TestVArray.h"
#include "Clownfish/Test/Util/TestAtomic.h"
#include "Clownfish/Test/Util/TestMemory.h"
#include "Clownfish/Test/Util/TestNumberUtils.h"
#include "Clownfish/Test/Util/TestStringHelper.h"

static void
S_unbuffer_stdout();

static VArray*
S_all_test_batches(TestFormatter *formatter) {
    VArray *batches = VA_new(0);

    VA_Push(batches, (Obj*)TestVArray_new(formatter));
    VA_Push(batches, (Obj*)TestHash_new(formatter));
    VA_Push(batches, (Obj*)TestObj_new(formatter));
    VA_Push(batches, (Obj*)TestErr_new(formatter));
    VA_Push(batches, (Obj*)TestBB_new(formatter));
    VA_Push(batches, (Obj*)TestCB_new(formatter));
    VA_Push(batches, (Obj*)TestNumUtil_new(formatter));
    VA_Push(batches, (Obj*)TestNum_new(formatter));
    VA_Push(batches, (Obj*)TestStrHelp_new(formatter));
    VA_Push(batches, (Obj*)TestAtomic_new(formatter));
    VA_Push(batches, (Obj*)TestLFReg_new(formatter));
    VA_Push(batches, (Obj*)TestMemory_new(formatter));

    return batches;
}

bool
Test_run_batch(CharBuf *class_name, TestFormatter *formatter) {
    S_unbuffer_stdout();

    VArray   *batches = S_all_test_batches(formatter);
    uint32_t  size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);

        if (CB_Equals(TestBatch_Get_Class_Name(batch), (Obj*)class_name)) {
            bool result = TestBatch_Run(batch);
            DECREF(batches);
            return result;
        }
    }

    DECREF(batches);
    THROW(ERR, "Couldn't find test class '%o'", class_name);
    UNREACHABLE_RETURN(bool);
}

bool
Test_run_all_batches(TestFormatter *formatter) {
    S_unbuffer_stdout();

    TestRunner *runner  = TestRunner_new(formatter);
    VArray     *batches = S_all_test_batches(formatter);
    uint32_t    size    = VA_Get_Size(batches);

    for (uint32_t i = 0; i < size; ++i) {
        TestBatch *batch = (TestBatch*)VA_Fetch(batches, i);
        TestRunner_Run_Batch(runner, batch);
    }

    bool result = TestRunner_Finish(runner);

    DECREF(runner);
    DECREF(batches);
    return result;
}

static void
S_unbuffer_stdout() {
    int check_val = setvbuf(stdout, NULL, _IONBF, 0);
    if (check_val != 0) {
        fprintf(stderr, "Failed when trying to unbuffer stdout\n");
    }
}


