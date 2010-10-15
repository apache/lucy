#define C_KINO_TESTREQOPTQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestReqOptQuery.h"
#include "KinoSearch/Search/RequiredOptionalQuery.h"
#include "KinoSearch/Search/LeafQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    Query *a_leaf  = (Query*)TestUtils_make_leaf_query(NULL, "a");
    Query *b_leaf  = (Query*)TestUtils_make_leaf_query(NULL, "b");
    Query *c_leaf  = (Query*)TestUtils_make_leaf_query(NULL, "c");
    RequiredOptionalQuery *query = ReqOptQuery_new(a_leaf, b_leaf);
    RequiredOptionalQuery *kids_differ = ReqOptQuery_new(a_leaf, c_leaf);
    RequiredOptionalQuery *boost_differs = ReqOptQuery_new(a_leaf, b_leaf);
    Obj *dump = (Obj*)ReqOptQuery_Dump(query);
    RequiredOptionalQuery *clone 
        = (RequiredOptionalQuery*)Obj_Load(dump, dump);

    TEST_FALSE(batch, ReqOptQuery_Equals(query, (Obj*)kids_differ), 
        "Different kids spoil Equals");
    TEST_TRUE(batch, ReqOptQuery_Equals(query, (Obj*)boost_differs), 
        "Equals with identical boosts");
    ReqOptQuery_Set_Boost(boost_differs, 1.5);
    TEST_FALSE(batch, ReqOptQuery_Equals(query, (Obj*)boost_differs), 
        "Different boost spoils Equals");
    TEST_TRUE(batch, ReqOptQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");

    DECREF(a_leaf);
    DECREF(b_leaf);
    DECREF(c_leaf);
    DECREF(query);
    DECREF(kids_differ);
    DECREF(boost_differs);
    DECREF(dump);
    DECREF(clone);
}

void
TestReqOptQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(4);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


