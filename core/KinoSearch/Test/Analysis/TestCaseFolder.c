#define C_KINO_TESTCASEFOLDER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Analysis/TestCaseFolder.h"
#include "KinoSearch/Analysis/CaseFolder.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    CaseFolder *case_folder = CaseFolder_new();
    CaseFolder *other       = CaseFolder_new();
    Obj        *dump        = (Obj*)CaseFolder_Dump(case_folder);
    CaseFolder *clone       = (CaseFolder*)CaseFolder_Load(other, dump);

    TEST_TRUE(batch, CaseFolder_Equals(case_folder, (Obj*)other), "Equals");
    TEST_FALSE(batch, CaseFolder_Equals(case_folder, (Obj*)&EMPTY), "Not Equals");
    TEST_TRUE(batch, CaseFolder_Equals(case_folder, (Obj*)clone), 
        "Dump => Load round trip");

    DECREF(case_folder);
    DECREF(other);
    DECREF(dump);
    DECREF(clone);
}

void
TestCaseFolder_run_tests()
{
    TestBatch *batch = TestBatch_new(3);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);

    DECREF(batch);
}


/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

