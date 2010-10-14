#define C_KINO_TESTBLOBTYPE
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Plan/TestBlobType.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Plan/BlobType.h"
#include "KinoSearch/Analysis/Tokenizer.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    BlobType *type            = BlobType_new(true);
    Obj      *dump            = (Obj*)BlobType_Dump(type);
    Obj      *clone           = Obj_Load(dump, dump);
    Obj      *another_dump    = (Obj*)BlobType_Dump_For_Schema(type);
    BlobType *another_clone   = BlobType_load(NULL, another_dump);

    TEST_TRUE(batch, BlobType_Equals(type, (Obj*)clone), 
        "Dump => Load round trip");
    TEST_TRUE(batch, BlobType_Equals(type, (Obj*)another_clone), 
        "Dump_For_Schema => Load round trip");

    DECREF(type);
    DECREF(dump);
    DECREF(clone);
    DECREF(another_dump);
    DECREF(another_clone);
}

void
TestBlobType_run_tests()
{
    TestBatch *batch = TestBatch_new(2);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

