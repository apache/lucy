#include <stdio.h>
#include "Charmonizer/Test.h"

int main() {
    int total_tests   = 0;
    int total_passed  = 0;
    int total_failed  = 0;
    int total_skipped = 0;
    int i;
    chaz_TestBatch* batches[7];

    chaz_Test_init();

    batches[0] = chaz_TFuncMacro_prepare();
    batches[1] = chaz_THeaders_prepare();
    batches[2] = chaz_TIntegers_prepare();
    batches[3] = chaz_TLargeFiles_prepare();
    batches[4] = chaz_TUnusedVars_prepare();
    batches[5] = chaz_TVariadicMacros_prepare();
    batches[6] = NULL;
    
    /* loop through test functions, accumulating results */
    for (i = 0; batches[i] != NULL; i++) {
        chaz_TestBatch *batch = batches[i];
        batch->run_test(batch);
        total_tests    += batch->num_tests;
        total_passed   += batch->num_passed;
        total_failed   += batch->num_failed;
        total_skipped  += batch->num_skipped;
        batch->destroy(batch);
    }
    
    /* print totals */
    printf("=============================\n");
    printf("TOTAL TESTS:   %d\nTOTAL PASSED:  %d\nTOTAL FAILED:  %d\n"
        "TOTAL SKIPPED: %d\n", 
        total_tests, total_passed, total_failed, total_skipped);
    return 0;
}

