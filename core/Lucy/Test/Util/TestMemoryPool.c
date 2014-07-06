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

#define C_TESTLUCY_TESTMEMORYPOOL
#define C_LUCY_MEMORYPOOL
#define TESTLUCY_USE_SHORT_NAMES
#include "Lucy/Util/ToolSet.h"

#include "Clownfish/TestHarness/TestBatchRunner.h"
#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestMemoryPool.h"
#include "Lucy/Util/MemoryPool.h"

TestMemoryPool*
TestMemPool_new() {
    return (TestMemoryPool*)Class_Make_Obj(TESTMEMORYPOOL);
}

void
TestMemPool_Run_IMP(TestMemoryPool *self, TestBatchRunner *runner) {
    TestBatchRunner_Plan(runner, (TestBatch*)self, 5);

    MemoryPool *mem_pool = MemPool_new(0);
    MemoryPoolIVARS *const ivars = MemPool_IVARS(mem_pool);
    char *ptr_a, *ptr_b;

    ptr_a = (char*)MemPool_Grab(mem_pool, 10);
    size_t expected = sizeof(void*) == 8 ? 16 : 12;
    TEST_INT_EQ(runner, MemPool_Get_Consumed(mem_pool), expected,
                "Round up allocation to word size");
    ptr_b = (char*)MemPool_Grab(mem_pool, 10);
    TEST_INT_EQ(runner, MemPool_Get_Consumed(mem_pool), expected * 2,
                "Accumulate consumed.");

    ptr_a = ivars->buf;
    MemPool_Resize(mem_pool, ptr_b, 6);
    TEST_TRUE(runner, ivars->buf < ptr_a, "Resize adjusts next allocation");
    TEST_TRUE(runner, MemPool_Get_Consumed(mem_pool) < expected * 2,
                "Resize() adjusts `consumed`");

    MemPool_Release_All(mem_pool);
    TEST_INT_EQ(runner, MemPool_Get_Consumed(mem_pool), 0,
                "Release_All() resets `consumed`");

    DECREF(mem_pool);
}


