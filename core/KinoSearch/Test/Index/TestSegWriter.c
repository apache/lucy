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

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

