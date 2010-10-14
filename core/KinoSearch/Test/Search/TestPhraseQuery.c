#define C_KINO_TESTPHRASEQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestPhraseQuery.h"
#include "KinoSearch/Search/PhraseQuery.h"

static void
test_Dump_And_Load(TestBatch *batch)
{
    PhraseQuery *query 
        = TestUtils_make_phrase_query("content", "a", "b", "c", NULL);
    Obj         *dump  = (Obj*)PhraseQuery_Dump(query);
    PhraseQuery *evil_twin = (PhraseQuery*)Obj_Load(dump, dump);
    TEST_TRUE(batch, PhraseQuery_Equals(query, (Obj*)evil_twin), 
        "Dump => Load round trip");
    DECREF(query);
    DECREF(dump);
    DECREF(evil_twin);
}

void
TestPhraseQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(1);
    TestBatch_Plan(batch);
    test_Dump_And_Load(batch);
    DECREF(batch);
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

