#define C_KINO_TESTHIGHLIGHTWRITER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Index/TestHighlightWriter.h"
#include "KinoSearch/Index/HighlightWriter.h"

void
TestHLWriter_run_tests()
{
    TestBatch *batch = TestBatch_new(1);
    TestBatch_Plan(batch);
    PASS(batch, "Placeholder");
    DECREF(batch);
}


