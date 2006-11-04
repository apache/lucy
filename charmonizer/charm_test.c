#include <stdio.h>
#include "Charmonizer/Test.h"

/* this is the signature for all Charmonizer test functions */
typedef chaz_TestBatch*
(*t_func)();

/* create an array of test functions to loop through */
typedef struct TestGroup {
    const char *name;
    t_func func;
} TestGroup;
TestGroup tests[] = {
    { "FuncMacro", chaz_Test_test_FuncMacro },
    { "Headers", chaz_Test_test_Headers },
    { "Integers", chaz_Test_test_Integers },
    { "LargeFiles", chaz_Test_test_LargeFiles },
    { "UnusedVars", chaz_Test_test_UnusedVars },
    { "VariadicMacros", chaz_Test_test_VariadicMacros },
    { NULL, NULL }
};

int main() {
    int total_tests   = 0;
    int total_passed  = 0;
    int total_failed  = 0;
    int total_skipped = 0;
    int i;

    chaz_Test_init();
    
    /* loop through test functions, accumulating results */
    for (i = 0; tests[i].name != NULL; i++) {
        t_func test_func = tests[i].func;
        const char *name = tests[i].name;
        chaz_TestBatch *batch = test_func();
        printf("=========================\n");
        printf("%s\n=========================\n", name);
        total_tests    += batch->num_tests;
        total_passed   += batch->num_passed;
        total_failed   += batch->num_failed;
        total_skipped  += batch->num_skipped;
        printf("-------------------------\n");
        printf("Tests:   %d\nPassed:  %d\nFailed:  %d\nSkipped: %d\n\n",
            batch->num_tests, batch->num_passed, batch->num_failed, 
            batch->num_skipped);
        batch->destroy(batch);
    }
    
    /* print totals */
    printf("=============================\n");
    printf("TOTAL TESTS:   %d\nTOTAL PASSED:  %d\nTOTAL FAILED:  %d\n"
        "TOTAL SKIPPED: %d\n", 
        total_tests, total_passed, total_failed, total_skipped);
    return 0;
}

