#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Util/TestAtomic.h"
#include "KinoSearch/Util/Atomic.h"

static void
test_cas_ptr(TestBatch *batch)
{
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
TestAtomic_run_tests()
{
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);

    test_cas_ptr(batch);

    DECREF(batch);
}


