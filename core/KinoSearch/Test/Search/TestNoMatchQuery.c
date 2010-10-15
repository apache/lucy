#define C_KINO_TESTNOMATCHQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestNoMatchQuery.h"
#include "KinoSearch/Search/NoMatchQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    NoMatchQuery *query = NoMatchQuery_new();
    Obj          *dump  = (Obj*)NoMatchQuery_Dump(query);
    NoMatchQuery *clone = (NoMatchQuery*)NoMatchQuery_Load(query, dump);

    TEST_TRUE(batch, NoMatchQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");
    TEST_FALSE(batch, NoMatchQuery_Equals(query, (Obj*)&EMPTY), "Equals");

    DECREF(query);
    DECREF(dump);
    DECREF(clone);
}


void
TestNoMatchQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(2);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


