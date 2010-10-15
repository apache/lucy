#define CHAZ_USE_SHORT_NAMES

#include <stdlib.h>
#include <stdio.h>
#include "Charmonizer/Test/AllTests.h"
    
static TestBatch **batches = NULL;

void
AllTests_init()
{
    Test_init();

    /* Create a null-terminated array of test batches to iterate over. */
    batches = (TestBatch**)malloc(8 * sizeof(TestBatch*));
    batches[0] = TestDirManip_prepare();
    batches[1] = TestFuncMacro_prepare();
    batches[2] = TestHeaders_prepare();
    batches[3] = TestIntegers_prepare();
    batches[4] = TestLargeFiles_prepare();
    batches[5] = TestUnusedVars_prepare();
    batches[6] = TestVariadicMacros_prepare();
    batches[7] = NULL;
}

void
AllTests_run()
{
    int total_tests   = 0;
    int total_passed  = 0;
    int total_failed  = 0;
    int total_skipped = 0;
    int i;
    
    /* Sanity check. */
    if (batches == NULL) {
        fprintf(stderr, "Must call AllTests_init() first.");
        exit(1);
    }

    /* Loop through test functions, accumulating results. */
    for (i = 0; batches[i] != NULL; i++) {
        TestBatch *batch = batches[i];
        batch->run_test(batch);
        total_tests    += batch->num_tests;
        total_passed   += batch->num_passed;
        total_failed   += batch->num_failed;
        total_skipped  += batch->num_skipped;
        batch->destroy(batch);
    }
    
    /* Print totals. */
    printf("=============================\n");
    printf("TOTAL TESTS:   %d\nTOTAL PASSED:  %d\nTOTAL FAILED:  %d\n"
        "TOTAL SKIPPED: %d\n", 
        total_tests, total_passed, total_failed, total_skipped);
}



