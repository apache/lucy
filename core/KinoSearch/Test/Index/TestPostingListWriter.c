#define C_KINO_TESTPOSTINGLISTWRITER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Index/TestPostingListWriter.h"

void
TestPListWriter_run_tests()
{
    TestBatch *batch = TestBatch_new(1);
    TestBatch_Plan(batch);
    PASS(batch, "Placeholder");
    DECREF(batch);
}


