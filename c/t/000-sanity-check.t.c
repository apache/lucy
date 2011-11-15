#define CHAZ_USE_SHORT_NAMES

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Charmonizer/Test.h";
#include "Charmonizer/Test.c";

static void
S_run_tests(TestBatch *batch) {
    TEST_INT_EQ(batch, 0, 0, "sanity check");
}

int main(int argc, char **argv) {
    TestBatch *batch;
    
    Test_init();
    batch = Test_new_batch("sanity", 1, S_run_tests);
    batch->run_test(batch);
    batch->destroy(batch);
    return 0;
}
