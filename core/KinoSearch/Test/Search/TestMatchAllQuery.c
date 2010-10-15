#define C_KINO_TESTMATCHALLQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestMatchAllQuery.h"
#include "KinoSearch/Search/MatchAllQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    MatchAllQuery *query = MatchAllQuery_new();
    Obj           *dump  = (Obj*)MatchAllQuery_Dump(query);
    MatchAllQuery *clone = (MatchAllQuery*)MatchAllQuery_Load(query, dump);

    TEST_TRUE(batch, MatchAllQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");
    TEST_FALSE(batch, MatchAllQuery_Equals(query, (Obj*)&EMPTY), "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}


void
TestMatchAllQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(2);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


