#include <stdio.h>
#include "Charmonizer/Test/TestHandler.h"

/* this is the signature for all Charmonizer test functions */
typedef void
(*t_func)(int *num_tests, int *num_passed, int *num_failed, int *num_skipped);

/* create an array of test functions to loop through */
typedef struct TestGroup {
    const char *name;
    t_func func;
} TestGroup;
TestGroup tests[] = {
    { "FuncMacro", chaz_TestHand_test_FuncMacro },
    { "Integers", chaz_TestHand_test_Integers },
    { "LargeFiles", chaz_TestHand_test_LargeFiles },
    { "UnusedVars", chaz_TestHand_test_UnusedVars },
    { "VariadicMacros", chaz_TestHand_test_VariadicMacros },
    { NULL, NULL }
};

int main() {
    int num_tests, num_passed, num_failed, num_skipped;
    int total_tests   = 0;
    int total_passed  = 0;
    int total_failed  = 0;
    int total_skipped = 0;
    int i;

    /* loop through test functions, accumulating results */
    for (i = 0; tests[i].name != NULL; i++) {
        t_func test_func = tests[i].func;
        const char *name = tests[i].name;
        printf("=========================\n");
        printf("%s\n=========================\n", name);
        test_func(&num_tests, &num_passed, &num_failed, &num_skipped);
        total_tests    += num_tests;
        total_passed   += num_passed;
        total_failed   += num_failed;
        total_skipped  += num_skipped;
        printf("-------------------------\n");
        printf("Tests:   %d\nPassed:  %d\nFailed:  %d\nSkipped: %d\n\n",
            num_tests, num_passed, num_failed, num_skipped);
    }
    
    /* print totals */
    printf("=============================\n");
    printf("TOTAL TESTS:   %d\nTOTAL PASSED:  %d\nTOTAL FAILED:  %d\n"
        "TOTAL SKIPPED: %d\n", 
        total_tests, total_passed, total_failed, total_skipped);
    return 0;
}

