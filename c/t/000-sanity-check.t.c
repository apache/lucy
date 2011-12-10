#define CHAZ_USE_SHORT_NAMES

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "Charmonizer/Test.h";
#include "Charmonizer/Test.c";

static void
S_run_tests(void) {
    LONG_EQ(0, 0, "sanity check");
}

int main(int argc, char **argv) {
    Test_start(1);
    S_run_tests();
    return !Test_finish();
}

