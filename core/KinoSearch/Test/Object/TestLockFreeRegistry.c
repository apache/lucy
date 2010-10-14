#include <string.h>

#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Object/TestLockFreeRegistry.h"
#include "KinoSearch/Object/LockFreeRegistry.h"

StupidHashCharBuf*
StupidHashCharBuf_new(char *text)
{
    return (StupidHashCharBuf*)CB_new_from_utf8(text, strlen(text));
}

int32_t
StupidHashCharBuf_hash_sum(StupidHashCharBuf *self)
{
    UNUSED_VAR(self);
    return 1;
}

static void
test_all(TestBatch *batch)
{
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
TestLFReg_run_tests()
{
    TestBatch *batch = TestBatch_new(6);

    TestBatch_Plan(batch);
    test_all(batch);

    DECREF(batch);
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

