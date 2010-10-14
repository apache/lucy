#define C_KINO_TESTLEAFQUERY
#include "KinoSearch/Util/ToolSet.h"
#include <math.h>

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Search/TestLeafQuery.h"
#include "KinoSearch/Test/TestUtils.h"
#include "KinoSearch/Search/LeafQuery.h"

static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    LeafQuery *query         = TestUtils_make_leaf_query("content", "foo");
    LeafQuery *field_differs = TestUtils_make_leaf_query("stuff", "foo");
    LeafQuery *null_field    = TestUtils_make_leaf_query(NULL, "foo");
    LeafQuery *term_differs  = TestUtils_make_leaf_query("content", "bar");
    LeafQuery *boost_differs = TestUtils_make_leaf_query("content", "foo");
    Obj       *dump          = (Obj*)LeafQuery_Dump(query);
    LeafQuery *clone         = (LeafQuery*)LeafQuery_Load(term_differs, dump);

    TEST_FALSE(batch, LeafQuery_Equals(query, (Obj*)field_differs),
        "Equals() false with different field");
    TEST_FALSE(batch, LeafQuery_Equals(query, (Obj*)null_field),
        "Equals() false with null field");
    TEST_FALSE(batch, LeafQuery_Equals(query, (Obj*)term_differs),
        "Equals() false with different term");
    LeafQuery_Set_Boost(boost_differs, 0.5);
    TEST_FALSE(batch, LeafQuery_Equals(query, (Obj*)boost_differs),
        "Equals() false with different boost");
    TEST_TRUE(batch, LeafQuery_Equals(query, (Obj*)clone), 
        "Dump => Load round trip");

    DECREF(query);
    DECREF(term_differs);
    DECREF(field_differs);
    DECREF(null_field);
    DECREF(boost_differs);
    DECREF(dump);
    DECREF(clone);
}

void
TestLeafQuery_run_tests()
{
    TestBatch *batch = TestBatch_new(5);
    TestBatch_Plan(batch);
    test_Dump_Load_and_Equals(batch);
    DECREF(batch);
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

