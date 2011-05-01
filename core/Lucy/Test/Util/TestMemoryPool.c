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

#define C_LUCY_TESTMEMORYPOOL
#define C_LUCY_MEMORYPOOL
#include "Lucy/Util/ToolSet.h"

#include "Lucy/Test.h"
#include "Lucy/Test/Util/TestMemoryPool.h"
#include "Lucy/Util/MemoryPool.h"

void
TestMemPool_run_tests() {
    TestBatch  *batch     = TestBatch_new(4);
    MemoryPool *mem_pool  = MemPool_new(0);
    MemoryPool *other     = MemPool_new(0);
    char *ptr_a, *ptr_b;

    TestBatch_Plan(batch);

    ptr_a = (char*)MemPool_Grab(mem_pool, 10);
    strcpy(ptr_a, "foo");
    MemPool_Release_All(mem_pool);

    ptr_b = (char*)MemPool_Grab(mem_pool, 10);
    TEST_STR_EQ(batch, ptr_b, "foo", "Recycle RAM on Release_All");

    ptr_a = mem_pool->buf;
    MemPool_Resize(mem_pool, ptr_b, 6);
    TEST_TRUE(batch, mem_pool->buf < ptr_a, "Resize");

    ptr_a = (char*)MemPool_Grab(other, 20);
    MemPool_Release_All(other);
    MemPool_Eat(other, mem_pool);
    TEST_TRUE(batch, other->buf == mem_pool->buf, "Eat");
    TEST_TRUE(batch, other->buf != NULL, "Eat");

    DECREF(mem_pool);
    DECREF(other);
    DECREF(batch);
}


