#define CHAZ_USE_SHORT_NAMES

#include "charmony.h"
#include "Charmonizer/Test.h"
#include "Charmonizer/Test/AllTests.h"

TestBatch*
TestUnusedVars_prepare()
{
    return Test_new_batch("UnusedVars", 2, TestUnusedVars_run);
}
void
TestUnusedVars_run(TestBatch *batch)
{
#ifdef UNUSED_VAR
    PASS(batch, "UNUSED_VAR macro is defined");
#else
    FAIL(batch, "UNUSED_VAR macro is defined");
#endif

#ifdef UNREACHABLE_RETURN
    PASS(batch, "UNREACHABLE_RETURN macro is defined");
#else
    FAIL(batch, "UNREACHABLE_RETURN macro is defined");
#endif
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

