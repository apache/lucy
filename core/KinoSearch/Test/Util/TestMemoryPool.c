#define C_KINO_TESTMEMORYPOOL
#define C_KINO_MEMORYPOOL
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Util/TestMemoryPool.h"
#include "KinoSearch/Util/MemoryPool.h"

void
TestMemPool_run_tests()
{
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


