#define C_KINO_TESTSEGWRITER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Test.h"
#include "KinoSearch/Test/Index/TestSegWriter.h"
#include "KinoSearch/Index/SegWriter.h"

void
TestSegWriter_run_tests()
{
    TestBatch *batch = TestBatch_new(1);
    TestBatch_Plan(batch);
    PASS(batch, "placeholder");
    DECREF(batch);
}


