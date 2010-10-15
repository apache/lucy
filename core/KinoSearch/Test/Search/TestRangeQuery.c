#define C_KINO_TESTRANGEQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Test/Search/TestRangeQuery.h"
#include "KinoSearch/Search/RangeQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    RangeQuery *query = TestUtils_make_range_query("content", "foo", "phooey",
        true, true);
    RangeQuery *lo_term_differs = TestUtils_make_range_query("content", 
        "goo", "phooey", true, true);
    RangeQuery *hi_term_differs = TestUtils_make_range_query("content", 
        "foo", "gooey", true, true);
    RangeQuery *include_lower_differs = TestUtils_make_range_query("content", 
        "foo", "phooey", false, true);
    RangeQuery *include_upper_differs = TestUtils_make_range_query("content", 
        "foo", "phooey", true, false);
    Obj        *dump  = (Obj*)RangeQuery_Dump(query);
    RangeQuery *clone = (RangeQuery*)RangeQuery_Load(lo_term_differs, dump);

    TEST_FALSE(batch, RangeQuery_Equals(query, (Obj*)lo_term_differs),
        "Equals() false with different lower term");
    TEST_FALSE(batch, RangeQuery_Equals(query, (Obj*)hi_term_differs),
        "Equals() false with different upper term");
    TEST_FALSE(batch, RangeQuery_Equals(query, (Obj*)include_lower_differs),
        "Equals() false with different include_lower");
    TEST_FALSE(batch, RangeQuery_Equals(query, (Obj*)include_upper_differs),
        "Equals() false with different include_upper");
    TEST_TRUE(batch, RangeQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");

    DECREF(query);
    DECREF(lo_term_differs);
    DECREF(hi_term_differs);
    DECREF(include_lower_differs);
    DECREF(include_upper_differs);
    DECREF(dump);
    DECREF(clone);
}


void
TestRangeQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(5);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}


