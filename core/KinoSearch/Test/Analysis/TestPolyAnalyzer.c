#define C_KINO_TESTPOLYANALYZER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Analysis/TestPolyAnalyzer.h"
#include "KinoSearch/Analysis/PolyAnalyzer.h"


static void
test_Dump_Load_and_Equals(TestBatch *batch)
{
    CharBuf      *EN          = (CharBuf*)ZCB_WRAP_STR("en", 2);
    CharBuf      *ES          = (CharBuf*)ZCB_WRAP_STR("es", 2);
    PolyAnalyzer *analyzer    = PolyAnalyzer_new(EN, NULL);
    PolyAnalyzer *other       = PolyAnalyzer_new(ES, NULL);
    Obj          *dump        = (Obj*)PolyAnalyzer_Dump(analyzer);
    Obj          *other_dump  = (Obj*)PolyAnalyzer_Dump(other);
    PolyAnalyzer *clone       = (PolyAnalyzer*)PolyAnalyzer_Load(other, dump);
    PolyAnalyzer *other_clone 
        = (PolyAnalyzer*)PolyAnalyzer_Load(other, other_dump);

    TEST_FALSE(batch, PolyAnalyzer_Equals(analyzer,
        (Obj*)other), "Equals() false with different language");
    TEST_TRUE(batch, PolyAnalyzer_Equals(analyzer,
        (Obj*)clone), "Dump => Load round trip");
    TEST_TRUE(batch, PolyAnalyzer_Equals(other,
        (Obj*)other_clone), "Dump => Load round trip");

    DECREF(analyzer);
    DECREF(dump);
    DECREF(clone);
    DECREF(other);
    DECREF(other_dump);
    DECREF(other_clone);
}

void
TestPolyAnalyzer_run_tests()
{
    TestBatch *batch = TestBatch_new(3);

    TestBatch_Plan(batch);

    test_Dump_Load_and_Equals(batch);

    DECREF(batch);
}



