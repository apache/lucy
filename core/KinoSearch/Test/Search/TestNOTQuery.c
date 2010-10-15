#define C_KINO_TESTNOTQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestNOTQuery.h"
#include "KinoSearch/Search/NOTQuery.h"
#include "KinoSearch/Search/LeafQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    Query    *a_leaf        = (Query*)TestUtils_make_leaf_query(NULL, "a");
    Query    *b_leaf        = (Query*)TestUtils_make_leaf_query(NULL, "b");
    NOTQuery *query         = NOTQuery_new(a_leaf);
    NOTQuery *kids_differ   = NOTQuery_new(b_leaf);
    NOTQuery *boost_differs = NOTQuery_new(a_leaf);
    Obj      *dump          = (Obj*)NOTQuery_Dump(query);
    NOTQuery *clone         = (NOTQuery*)Obj_Load(dump, dump);

    TEST_FALSE(batch, NOTQuery_Equals(query, (Obj*)kids_differ), 
        "Different kids spoil Equals");
    TEST_TRUE(batch, NOTQuery_Equals(query, (Obj*)boost_differs), 
        "Equals with identical boosts");
    NOTQuery_Set_Boost(boost_differs, 1.5);
    TEST_FALSE(batch, NOTQuery_Equals(query, (Obj*)boost_differs), 
        "Different boost spoils Equals");
    TEST_TRUE(batch, NOTQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");

    DECREF(a_leaf);
    DECREF(b_leaf);
    DECREF(query);
    DECREF(kids_differ);
    DECREF(boost_differs);
    DECREF(dump);
    DECREF(clone);
}

void
TestNOTQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(4);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


